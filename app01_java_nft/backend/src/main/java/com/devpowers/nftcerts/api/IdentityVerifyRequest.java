package com.devpowers.nftcerts.api;

import jakarta.validation.constraints.NotBlank;

public record IdentityVerifyRequest(@NotBlank String walletAddress, @NotBlank String did, String email) {
}
