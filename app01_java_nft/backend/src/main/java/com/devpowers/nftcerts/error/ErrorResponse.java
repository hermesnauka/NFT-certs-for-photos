package com.devpowers.nftcerts.error;

import java.time.Instant;

/** Uniform error body shape returned by {@link GlobalExceptionHandler} for all mapped errors. */
public record ErrorResponse(Instant timestamp, int status, String error, String message, String path) {
}
