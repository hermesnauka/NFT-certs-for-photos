package com.devpowers.nftcerts.blockchain;

import java.math.BigInteger;

/** Result of a successful on-chain {@code mintCertificate} call. */
public record MintResult(BigInteger tokenId, String txHash, String contractAddress) {
}
