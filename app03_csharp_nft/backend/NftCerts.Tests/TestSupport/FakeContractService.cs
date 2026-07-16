using NftCerts.Blockchain;
using NftCerts.Config;
using NftCerts.Errors;

namespace NftCerts.Tests.TestSupport;

// Substitutes for ContractService in tests: no real JSON-RPC/chain access, matching NFR-4 (unit
// tests must not require live network access). ContractService's mint/read methods are `virtual`
// specifically so a test double can override them like this.
public class FakeContractService() : ContractService(new Web3Properties())
{
    private long _nextTokenId = 1;
    public HashSet<string> RegisteredHashes { get; } = [];
    public Func<string, string, string, string, long, MintResult>? MintOverride { get; set; }

    public override MintResult MintCertificate(string to, string metadataUri, string contentHashHex,
                                                 string royaltyRecipient, long royaltyFeeBasisPoints)
    {
        if (MintOverride is not null)
        {
            return MintOverride(to, metadataUri, contentHashHex, royaltyRecipient, royaltyFeeBasisPoints);
        }

        if (!RegisteredHashes.Add(contentHashHex))
        {
            throw new DuplicateContentHashException(contentHashHex);
        }

        return new MintResult(_nextTokenId++, "0x" + new string('a', 64), "0xFakeContractAddress0000000000000000");
    }

    public override string? TokenContentHash(long tokenId) => null;
}
