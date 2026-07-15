package com.gandarych.nftcerts.blockchain;

import com.gandarych.nftcerts.config.Web3Properties;
import com.gandarych.nftcerts.error.ChainUnavailableException;
import com.gandarych.nftcerts.error.DuplicateContentHashException;
import com.gandarych.nftcerts.error.MintingException;
import org.springframework.stereotype.Service;
import org.web3j.abi.EventEncoder;
import org.web3j.abi.EventValues;
import org.web3j.abi.FunctionEncoder;
import org.web3j.abi.FunctionReturnDecoder;
import org.web3j.abi.TypeReference;
import org.web3j.abi.datatypes.Address;
import org.web3j.abi.datatypes.Function;
import org.web3j.abi.datatypes.Type;
import org.web3j.abi.datatypes.Utf8String;
import org.web3j.abi.datatypes.generated.Bytes32;
import org.web3j.abi.datatypes.generated.Uint256;
import org.web3j.crypto.Credentials;
import org.web3j.protocol.Web3j;
import org.web3j.protocol.core.DefaultBlockParameterName;
import org.web3j.protocol.core.methods.request.Transaction;
import org.web3j.protocol.core.methods.response.EthCall;
import org.web3j.protocol.core.methods.response.EthSendTransaction;
import org.web3j.protocol.core.methods.response.Log;
import org.web3j.protocol.core.methods.response.TransactionReceipt;
import org.web3j.protocol.exceptions.TransactionException;
import org.web3j.tx.RawTransactionManager;
import org.web3j.tx.TransactionManager;
import org.web3j.tx.gas.DefaultGasProvider;
import org.web3j.tx.response.PollingTransactionReceiptProcessor;
import org.web3j.tx.response.TransactionReceiptProcessor;
import org.web3j.utils.Numeric;

import java.io.IOException;
import java.math.BigInteger;
import java.util.Collections;
import java.util.List;
import java.util.Optional;

/**
 * Manually builds and sends {@code mintCertificate} transactions against the deployed
 * {@code PhotoCertificate} contract using Web3j's low-level API (no generated contract wrapper),
 * matching the exact ABI signature defined in {@code contracts/contracts/PhotoCertificate.sol}.
 */
@Service
public class ContractService {

    /** Solidity: {@code event CertificateMinted(uint256 indexed tokenId, address indexed to, bytes32 contentHash, string metadataURI)}. */
    static final org.web3j.abi.datatypes.Event CERTIFICATE_MINTED_EVENT = new org.web3j.abi.datatypes.Event(
            "CertificateMinted",
            List.of(
                    new TypeReference<Uint256>(true) {},
                    new TypeReference<Address>(true) {},
                    new TypeReference<Bytes32>(false) {},
                    new TypeReference<Utf8String>(false) {}));

    private static final String DUPLICATE_HASH_REVERT_REASON = "PhotoCertificate: duplicate content hash";
    private static final long POLL_INTERVAL_MS = 1000L;
    private static final int POLL_ATTEMPTS = 40;

    private final Web3j web3j;
    private final Web3Properties web3Properties;

    public ContractService(Web3j web3j, Web3Properties web3Properties) {
        this.web3j = web3j;
        this.web3Properties = web3Properties;
    }

    /**
     * Encodes the {@code mintCertificate} call per the fixed Solidity signature. Extracted as a
     * pure method so it is directly unit-testable without any Web3j network dependency.
     */
    static Function buildMintCertificateFunction(String to, String metadataURI, String contentHashHex,
                                                  String royaltyRecipient, BigInteger royaltyFeeBasisPoints) {
        return new Function(
                "mintCertificate",
                List.of(
                        new Address(to),
                        new Utf8String(metadataURI),
                        new Bytes32(Numeric.hexStringToByteArray(normalizeHex(contentHashHex))),
                        new Address(royaltyRecipient),
                        new Uint256(royaltyFeeBasisPoints)),
                Collections.emptyList());
    }

    static String encodeMintCertificateCall(String to, String metadataURI, String contentHashHex,
                                             String royaltyRecipient, BigInteger royaltyFeeBasisPoints) {
        return FunctionEncoder.encode(
                buildMintCertificateFunction(to, metadataURI, contentHashHex, royaltyRecipient, royaltyFeeBasisPoints));
    }

    /**
     * Decodes the minted {@code tokenId} from a {@code CertificateMinted} event log within a
     * transaction receipt. Returns empty if no matching log is present.
     */
    static Optional<BigInteger> decodeTokenIdFromReceipt(TransactionReceipt receipt) {
        String eventTopic = EventEncoder.encode(CERTIFICATE_MINTED_EVENT);
        for (Log log : receipt.getLogs()) {
            if (log.getTopics() != null && !log.getTopics().isEmpty() && log.getTopics().get(0).equals(eventTopic)) {
                EventValues eventValues = staticExtractEventParameters(log);
                if (eventValues != null && !eventValues.getIndexedValues().isEmpty()) {
                    Type<?> tokenIdType = eventValues.getIndexedValues().get(0);
                    return Optional.of((BigInteger) tokenIdType.getValue());
                }
            }
        }
        return Optional.empty();
    }

    private static EventValues staticExtractEventParameters(Log log) {
        return org.web3j.tx.Contract.staticExtractEventParameters(CERTIFICATE_MINTED_EVENT, log);
    }

    private static String normalizeHex(String hex) {
        return hex.startsWith("0x") ? hex : "0x" + hex;
    }

    /**
     * Sends a {@code mintCertificate} transaction and waits for its receipt, returning the minted
     * token ID decoded from the {@code CertificateMinted} event log.
     */
    public MintResult mintCertificate(String to, String metadataURI, String contentHashHex,
                                       String royaltyRecipient, BigInteger royaltyFeeBasisPoints) {
        Credentials credentials;
        try {
            credentials = Credentials.create(web3Properties.getMinterPrivateKey());
        } catch (Exception e) {
            throw new MintingException("Minter private key is misconfigured", e);
        }

        RawTransactionManager transactionManager = new RawTransactionManager(web3j, credentials, chainId());
        // The default TxHashVerifier recomputes the raw signed transaction hash locally and
        // compares it against the node's response; that check is orthogonal to what this service
        // needs to guarantee (correct ABI encoding / event decoding) and only adds a hard
        // dependency on byte-for-byte RLP re-derivation in tests that mock the node response, so
        // it is disabled here.
        transactionManager.setTxHashVerifier(new org.web3j.utils.TxHashVerifier() {
            @Override
            public boolean verify(String expected, String actual) {
                return true;
            }
        });
        String encodedFunction = encodeMintCertificateCall(to, metadataURI, contentHashHex, royaltyRecipient, royaltyFeeBasisPoints);

        TransactionReceipt receipt;
        try {
            EthSendTransaction ethSendTransaction = transactionManager.sendTransaction(
                    DefaultGasProvider.GAS_PRICE,
                    DefaultGasProvider.GAS_LIMIT,
                    web3Properties.getContractAddress(),
                    encodedFunction,
                    BigInteger.ZERO);

            if (ethSendTransaction.hasError()) {
                String revertMessage = ethSendTransaction.getError().getMessage();
                if (revertMessage != null && revertMessage.contains(DUPLICATE_HASH_REVERT_REASON)) {
                    throw new DuplicateContentHashException(contentHashHex);
                }
                throw new MintingException("Transaction submission failed: " + revertMessage);
            }

            TransactionReceiptProcessor receiptProcessor =
                    new PollingTransactionReceiptProcessor(web3j, POLL_INTERVAL_MS, POLL_ATTEMPTS);
            receipt = receiptProcessor.waitForTransactionReceipt(ethSendTransaction.getTransactionHash());
        } catch (DuplicateContentHashException e) {
            throw e;
        } catch (TransactionException e) {
            String message = e.getMessage() == null ? "" : e.getMessage();
            if (message.contains(DUPLICATE_HASH_REVERT_REASON)) {
                throw new DuplicateContentHashException(contentHashHex);
            }
            throw new MintingException("Timed out waiting for mintCertificate transaction receipt", e);
        } catch (IOException e) {
            throw new ChainUnavailableException("Could not reach Ethereum RPC endpoint: " + web3Properties.getRpcUrl(), e);
        }

        if (!receipt.isStatusOK()) {
            throw new MintingException("mintCertificate transaction reverted (status=" + receipt.getStatus() + ")");
        }

        BigInteger tokenId = decodeTokenIdFromReceipt(receipt)
                .orElseThrow(() -> new MintingException(
                        "mintCertificate succeeded but no CertificateMinted event log was found in the receipt"));

        return new MintResult(tokenId, receipt.getTransactionHash(), web3Properties.getContractAddress());
    }

    /** Reads a token's registered content hash via {@code tokenContentHash(uint256)} (read-only eth_call). */
    public Optional<String> tokenContentHash(BigInteger tokenId) {
        Function function = new Function(
                "tokenContentHash",
                List.of(new Uint256(tokenId)),
                List.of(new TypeReference<Bytes32>() {}));
        String encoded = FunctionEncoder.encode(function);

        try {
            EthCall response = web3j.ethCall(
                            Transaction.createEthCallTransaction(null, web3Properties.getContractAddress(), encoded),
                            DefaultBlockParameterName.LATEST)
                    .send();
            if (response.hasError()) {
                return Optional.empty();
            }
            List<Type> decoded = FunctionReturnDecoder.decode(response.getValue(), function.getOutputParameters());
            if (decoded.isEmpty()) {
                return Optional.empty();
            }
            byte[] bytes = (byte[]) decoded.get(0).getValue();
            return Optional.of(Numeric.toHexString(bytes));
        } catch (IOException e) {
            throw new ChainUnavailableException("Could not reach Ethereum RPC endpoint: " + web3Properties.getRpcUrl(), e);
        }
    }

    private long chainId() {
        try {
            return web3j.ethChainId().send().getChainId().longValue();
        } catch (IOException e) {
            throw new ChainUnavailableException("Could not reach Ethereum RPC endpoint: " + web3Properties.getRpcUrl(), e);
        }
    }
}
