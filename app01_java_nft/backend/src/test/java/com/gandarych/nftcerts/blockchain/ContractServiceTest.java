package com.gandarych.nftcerts.blockchain;

import com.gandarych.nftcerts.config.Web3Properties;
import com.gandarych.nftcerts.error.MintingException;
import org.junit.jupiter.api.Test;
import org.web3j.abi.EventEncoder;
import org.web3j.abi.datatypes.Address;
import org.web3j.abi.datatypes.Function;
import org.web3j.abi.datatypes.Utf8String;
import org.web3j.abi.datatypes.generated.Bytes32;
import org.web3j.abi.datatypes.generated.Uint256;
import org.web3j.protocol.Web3j;
import org.web3j.protocol.core.Request;
import org.web3j.protocol.core.methods.response.EthChainId;
import org.web3j.protocol.core.methods.response.EthGetTransactionCount;
import org.web3j.protocol.core.methods.response.EthSendTransaction;
import org.web3j.protocol.core.methods.response.Log;
import org.web3j.protocol.core.methods.response.TransactionReceipt;
import org.web3j.utils.Numeric;

import java.math.BigInteger;
import java.util.List;
import java.util.Optional;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

class ContractServiceTest {

    private static final String CONTRACT_ADDRESS = "0x5FbDB2315678afecb367f032d93F642f64180aa";
    private static final String RECIPIENT = "0x70997970C51812dc3A010C7d01b50e0d17dc79C";
    private static final String ROYALTY_RECIPIENT = "0x70997970C51812dc3A010C7d01b50e0d17dc79C";
    private static final String CONTENT_HASH =
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    private static final String METADATA_URI = "ipfs://QmMetadata";

    @Test
    void buildsWellFormedAbiEncodedFunctionCall() {
        String encoded = ContractService.encodeMintCertificateCall(
                RECIPIENT, METADATA_URI, CONTENT_HASH, ROYALTY_RECIPIENT, BigInteger.valueOf(750));

        // 4-byte selector + at least 5 * 32-byte encoded head words
        assertTrue(encoded.startsWith("0x"));
        int hexBodyLength = encoded.length() - 2;
        assertTrue(hexBodyLength >= 8 + 5 * 64);

        String expectedSelector = org.web3j.abi.FunctionEncoder.encode(
                ContractService.buildMintCertificateFunction(RECIPIENT, METADATA_URI, CONTENT_HASH, ROYALTY_RECIPIENT, BigInteger.valueOf(750)))
                .substring(0, 10);
        assertEquals(expectedSelector, encoded.substring(0, 10));
    }

    @Test
    void functionHasExpectedNameAndInputTypes() {
        Function function = ContractService.buildMintCertificateFunction(
                RECIPIENT, METADATA_URI, CONTENT_HASH, ROYALTY_RECIPIENT, BigInteger.valueOf(750));

        assertEquals("mintCertificate", function.getName());
        assertEquals(5, function.getInputParameters().size());
        assertTrue(function.getInputParameters().get(0) instanceof Address);
        assertTrue(function.getInputParameters().get(1) instanceof Utf8String);
        assertTrue(function.getInputParameters().get(2) instanceof Bytes32);
        assertTrue(function.getInputParameters().get(3) instanceof Address);
        assertTrue(function.getInputParameters().get(4) instanceof Uint256);
    }

    @Test
    void decodesTokenIdFromMatchingCertificateMintedEventLog() {
        BigInteger expectedTokenId = BigInteger.valueOf(42);
        TransactionReceipt receipt = new TransactionReceipt();
        receipt.setLogs(List.of(buildCertificateMintedLog(expectedTokenId, RECIPIENT, CONTENT_HASH, METADATA_URI)));

        Optional<BigInteger> tokenId = ContractService.decodeTokenIdFromReceipt(receipt);

        assertTrue(tokenId.isPresent());
        assertEquals(expectedTokenId, tokenId.get());
    }

    @Test
    void decodeReturnsEmptyWhenNoMatchingEventLogPresent() {
        TransactionReceipt receipt = new TransactionReceipt();
        Log unrelatedLog = new Log();
        unrelatedLog.setTopics(List.of("0xdeadbeef"));
        receipt.setLogs(List.of(unrelatedLog));

        Optional<BigInteger> tokenId = ContractService.decodeTokenIdFromReceipt(receipt);

        assertTrue(tokenId.isEmpty());
    }

    @Test
    void mintCertificateThrowsMintingExceptionWhenNoEventLogFoundInSuccessfulReceipt() throws Exception {
        Web3j web3j = mock(Web3j.class);
        Web3Properties web3Properties = new Web3Properties();
        web3Properties.setContractAddress(CONTRACT_ADDRESS);
        web3Properties.setMinterPrivateKey("0x59c6995e998f97a5a0044966f0945389dc9e86dae88c7a8412f4603b6b78690");
        web3Properties.setRpcUrl("http://localhost:8545");

        mockChainIdAndNonce(web3j);

        EthSendTransaction sendTransaction = mock(EthSendTransaction.class);
        when(sendTransaction.hasError()).thenReturn(false);
        when(sendTransaction.getTransactionHash()).thenReturn("0xtxhash");
        Request<?, EthSendTransaction> sendRequest = mock(Request.class);
        when(sendRequest.send()).thenReturn(sendTransaction);
        when(web3j.ethSendRawTransaction(any())).thenReturn((Request) sendRequest);

        TransactionReceipt receiptWithNoEvents = new TransactionReceipt();
        receiptWithNoEvents.setStatus("0x1");
        receiptWithNoEvents.setTransactionHash("0xtxhash");
        receiptWithNoEvents.setLogs(List.of());

        org.web3j.protocol.core.methods.response.EthGetTransactionReceipt receiptResponse =
                mock(org.web3j.protocol.core.methods.response.EthGetTransactionReceipt.class);
        when(receiptResponse.getTransactionReceipt()).thenReturn(Optional.of(receiptWithNoEvents));
        Request<?, org.web3j.protocol.core.methods.response.EthGetTransactionReceipt> receiptRequest = mock(Request.class);
        when(receiptRequest.send()).thenReturn(receiptResponse);
        when(web3j.ethGetTransactionReceipt(any())).thenReturn((Request) receiptRequest);

        ContractService contractService = new ContractService(web3j, web3Properties);

        MintingException exception = org.junit.jupiter.api.Assertions.assertThrows(MintingException.class,
                () -> contractService.mintCertificate(RECIPIENT, METADATA_URI, CONTENT_HASH, ROYALTY_RECIPIENT, BigInteger.valueOf(750)));

        assertTrue(exception.getMessage().contains("CertificateMinted"));
    }

    @Test
    void mintCertificateReturnsTokenIdTxHashAndContractAddressOnSuccess() throws Exception {
        Web3j web3j = mock(Web3j.class);
        Web3Properties web3Properties = new Web3Properties();
        web3Properties.setContractAddress(CONTRACT_ADDRESS);
        web3Properties.setMinterPrivateKey("0x59c6995e998f97a5a0044966f0945389dc9e86dae88c7a8412f4603b6b78690");
        web3Properties.setRpcUrl("http://localhost:8545");

        mockChainIdAndNonce(web3j);

        EthSendTransaction sendTransaction = mock(EthSendTransaction.class);
        when(sendTransaction.hasError()).thenReturn(false);
        when(sendTransaction.getTransactionHash()).thenReturn("0xtxhash");
        Request<?, EthSendTransaction> sendRequest = mock(Request.class);
        when(sendRequest.send()).thenReturn(sendTransaction);
        when(web3j.ethSendRawTransaction(any())).thenReturn((Request) sendRequest);

        BigInteger expectedTokenId = BigInteger.valueOf(7);
        TransactionReceipt receipt = new TransactionReceipt();
        receipt.setStatus("0x1");
        receipt.setTransactionHash("0xtxhash");
        receipt.setLogs(List.of(buildCertificateMintedLog(expectedTokenId, RECIPIENT, CONTENT_HASH, METADATA_URI)));

        org.web3j.protocol.core.methods.response.EthGetTransactionReceipt receiptResponse =
                mock(org.web3j.protocol.core.methods.response.EthGetTransactionReceipt.class);
        when(receiptResponse.getTransactionReceipt()).thenReturn(Optional.of(receipt));
        Request<?, org.web3j.protocol.core.methods.response.EthGetTransactionReceipt> receiptRequest = mock(Request.class);
        when(receiptRequest.send()).thenReturn(receiptResponse);
        when(web3j.ethGetTransactionReceipt(any())).thenReturn((Request) receiptRequest);

        ContractService contractService = new ContractService(web3j, web3Properties);

        MintResult result = contractService.mintCertificate(RECIPIENT, METADATA_URI, CONTENT_HASH, ROYALTY_RECIPIENT, BigInteger.valueOf(750));

        assertEquals(expectedTokenId, result.tokenId());
        assertEquals("0xtxhash", result.txHash());
        assertEquals(CONTRACT_ADDRESS, result.contractAddress());
    }

    @Test
    void mintCertificateMapsDuplicateContentHashRevertTo409StyleException() throws Exception {
        Web3j web3j = mock(Web3j.class);
        Web3Properties web3Properties = new Web3Properties();
        web3Properties.setContractAddress(CONTRACT_ADDRESS);
        web3Properties.setMinterPrivateKey("0x59c6995e998f97a5a0044966f0945389dc9e86dae88c7a8412f4603b6b78690");
        web3Properties.setRpcUrl("http://localhost:8545");

        mockChainIdAndNonce(web3j);

        EthSendTransaction sendTransaction = mock(EthSendTransaction.class);
        when(sendTransaction.hasError()).thenReturn(true);
        org.web3j.protocol.core.Response.Error error = new org.web3j.protocol.core.Response.Error(
                3, "execution reverted: PhotoCertificate: duplicate content hash");
        when(sendTransaction.getError()).thenReturn(error);
        Request<?, EthSendTransaction> sendRequest = mock(Request.class);
        when(sendRequest.send()).thenReturn(sendTransaction);
        when(web3j.ethSendRawTransaction(any())).thenReturn((Request) sendRequest);

        ContractService contractService = new ContractService(web3j, web3Properties);

        com.gandarych.nftcerts.error.DuplicateContentHashException exception =
                org.junit.jupiter.api.Assertions.assertThrows(
                        com.gandarych.nftcerts.error.DuplicateContentHashException.class,
                        () -> contractService.mintCertificate(RECIPIENT, METADATA_URI, CONTENT_HASH, ROYALTY_RECIPIENT, BigInteger.valueOf(750)));

        assertTrue(exception.getMessage().contains(CONTENT_HASH));
    }

    private void mockChainIdAndNonce(Web3j web3j) throws Exception {
        EthChainId ethChainId = mock(EthChainId.class);
        when(ethChainId.getChainId()).thenReturn(BigInteger.valueOf(31337));
        Request<?, EthChainId> chainIdRequest = mock(Request.class);
        when(chainIdRequest.send()).thenReturn(ethChainId);
        when(web3j.ethChainId()).thenReturn((Request) chainIdRequest);

        EthGetTransactionCount txCount = mock(EthGetTransactionCount.class);
        when(txCount.getTransactionCount()).thenReturn(BigInteger.ZERO);
        Request<?, EthGetTransactionCount> txCountRequest = mock(Request.class);
        when(txCountRequest.send()).thenReturn(txCount);
        when(web3j.ethGetTransactionCount(any(), any())).thenReturn((Request) txCountRequest);
    }

    private Log buildCertificateMintedLog(BigInteger tokenId, String to, String contentHashHex, String metadataUri) {
        org.web3j.abi.datatypes.Event event = ContractService.CERTIFICATE_MINTED_EVENT;
        String eventTopic = EventEncoder.encode(event);

        String tokenIdTopic = Numeric.toHexStringWithPrefixZeroPadded(tokenId, 64);
        String toTopic = "0x" + "0".repeat(24) + to.substring(2).toLowerCase();

        // encode data section: bytes32 contentHash + dynamic string metadataURI (offset + length + padded bytes)
        String encodedData = org.web3j.abi.FunctionEncoder.encodeConstructor(
                List.of(new Bytes32(Numeric.hexStringToByteArray(contentHashHex)), new Utf8String(metadataUri)));

        Log log = new Log();
        log.setTopics(List.of(eventTopic, tokenIdTopic, toTopic));
        log.setData(encodedData);
        return log;
    }
}
