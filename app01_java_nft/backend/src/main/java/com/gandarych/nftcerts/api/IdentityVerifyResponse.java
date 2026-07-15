package com.gandarych.nftcerts.api;

public record IdentityVerifyResponse(boolean verified, String did, String walletAddress) {
}
