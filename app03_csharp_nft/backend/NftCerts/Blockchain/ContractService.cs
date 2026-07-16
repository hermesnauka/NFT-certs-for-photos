using System.Numerics;
using Nethereum.ABI.FunctionEncoding.Attributes;
using Nethereum.Contracts;
using Nethereum.JsonRpc.Client;
using Nethereum.RPC.Eth.DTOs;
using Nethereum.Web3;
using Nethereum.Web3.Accounts;
using NftCerts.Config;
using NftCerts.Errors;

namespace NftCerts.Blockchain;

public record MintResult(long TokenId, string TxHash, string ContractAddress);

// Solidity: mintCertificate(address to, string metadataURI, bytes32 contentHash,
//                            address royaltyRecipient, uint256 royaltyFeeBasisPoints)
[Function("mintCertificate", "uint256")]
public class MintCertificateFunction : FunctionMessage
{
    [Parameter("address", "to", 1)] public string To { get; set; } = "";
    [Parameter("string", "metadataURI", 2)] public string MetadataUri { get; set; } = "";
    [Parameter("bytes32", "contentHash", 3)] public byte[] ContentHash { get; set; } = [];
    [Parameter("address", "royaltyRecipient", 4)] public string RoyaltyRecipient { get; set; } = "";
    [Parameter("uint256", "royaltyFeeBasisPoints", 5)] public BigInteger RoyaltyFeeBasisPoints { get; set; }
}

// Solidity: event CertificateMinted(uint256 indexed tokenId, address indexed to,
//                                    bytes32 contentHash, string metadataURI)
[Event("CertificateMinted")]
public class CertificateMintedEventDto : IEventDTO
{
    [Parameter("uint256", "tokenId", 1, true)] public BigInteger TokenId { get; set; }
    [Parameter("address", "to", 2, true)] public string To { get; set; } = "";
    [Parameter("bytes32", "contentHash", 3, false)] public byte[] ContentHash { get; set; } = [];
    [Parameter("string", "metadataURI", 4, false)] public string MetadataUri { get; set; } = "";
}

[Function("tokenContentHash", "bytes32")]
public class TokenContentHashFunction : FunctionMessage
{
    [Parameter("uint256", "tokenId", 1)] public BigInteger TokenId { get; set; }
}

// Builds and sends mintCertificate transactions against the deployed PhotoCertificate contract
// via Nethereum — the C# equivalent of app01's Web3j-based ContractService (app02 hand-rolled the
// whole Ethereum stack instead; here we mirror the reference implementation's library choice).
public class ContractService(Web3Properties web3Properties)
{
    private const string DuplicateHashRevertReason = "PhotoCertificate: duplicate content hash";

    public virtual MintResult MintCertificate(string to, string metadataUri, string contentHashHex,
                                               string royaltyRecipient, long royaltyFeeBasisPoints)
    {
        Account account;
        try
        {
            account = new Account(web3Properties.MinterPrivateKey);
        }
        catch (Exception exception)
        {
            throw new MintingException($"Minter private key is misconfigured: {exception.Message}");
        }

        var web3 = new Web3(account, web3Properties.RpcUrl);
        var message = new MintCertificateFunction
        {
            To = to,
            MetadataUri = metadataUri,
            ContentHash = HexToBytes32(contentHashHex),
            RoyaltyRecipient = royaltyRecipient,
            RoyaltyFeeBasisPoints = royaltyFeeBasisPoints,
            Gas = 4_300_000,                                    // matches Web3j's DefaultGasProvider
            GasPrice = BigInteger.Parse("20000000000"),         // 20 gwei
        };

        TransactionReceipt receipt;
        try
        {
            var handler = web3.Eth.GetContractTransactionHandler<MintCertificateFunction>();
            receipt = handler.SendRequestAndWaitForReceiptAsync(web3Properties.ContractAddress, message)
                .GetAwaiter().GetResult();
        }
        catch (Exception exception) when (exception is RpcResponseException)
        {
            if (exception.Message.Contains(DuplicateHashRevertReason))
            {
                throw new DuplicateContentHashException(contentHashHex);
            }
            throw new MintingException($"Transaction submission failed: {exception.Message}");
        }
        catch (Exception exception) when (exception is HttpRequestException or RpcClientUnknownException)
        {
            throw new ChainUnavailableException(
                $"Could not reach Ethereum RPC endpoint: {web3Properties.RpcUrl}");
        }

        if (receipt.Status.Value != BigInteger.One)
        {
            throw new MintingException($"mintCertificate transaction reverted (status=0x{receipt.Status.Value:x})");
        }

        var events = receipt.DecodeAllEvents<CertificateMintedEventDto>();
        if (events.Count == 0)
        {
            throw new MintingException(
                "mintCertificate succeeded but no CertificateMinted event log was found in the receipt");
        }

        return new MintResult((long)events[0].Event.TokenId, receipt.TransactionHash,
                               web3Properties.ContractAddress);
    }

    // Reads a token's registered content hash via tokenContentHash(uint256) (read-only eth_call).
    public virtual string? TokenContentHash(long tokenId)
    {
        var web3 = new Web3(web3Properties.RpcUrl);
        var handler = web3.Eth.GetContractQueryHandler<TokenContentHashFunction>();
        byte[] result = handler
            .QueryAsync<byte[]>(web3Properties.ContractAddress, new TokenContentHashFunction { TokenId = tokenId })
            .GetAwaiter().GetResult();
        if (result.Length == 0 || result.All(b => b == 0))
        {
            return null;
        }
        return "0x" + Convert.ToHexStringLower(result);
    }

    internal static byte[] HexToBytes32(string hex)
    {
        string stripped = hex.StartsWith("0x", StringComparison.OrdinalIgnoreCase) ? hex[2..] : hex;
        if (stripped.Length != 64)
        {
            throw new MintingException($"contentHash must be exactly 32 bytes of hex, got {stripped.Length} chars");
        }
        return Convert.FromHexString(stripped);
    }
}
