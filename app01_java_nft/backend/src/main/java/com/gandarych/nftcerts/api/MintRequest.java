package com.gandarych.nftcerts.api;

import jakarta.validation.constraints.NotBlank;

public record MintRequest(@NotBlank String recipientAddress) {
}
