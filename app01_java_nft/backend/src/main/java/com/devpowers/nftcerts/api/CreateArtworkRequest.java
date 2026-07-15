package com.devpowers.nftcerts.api;

import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.NotNull;

import java.util.UUID;

public record CreateArtworkRequest(
        @NotNull UUID fileId,
        @NotBlank String title,
        String description,
        String medium,
        Integer yearCreated,
        @NotNull Integer royaltyPercentageBps,
        @NotBlank String artistWalletAddress,
        String artistDid
) {
}
