package com.gandarych.nftcerts.error;

import org.springframework.http.HttpStatus;

/**
 * Thrown for minting failures that are not specifically a duplicate-content-hash revert (e.g.
 * minter misconfiguration, unexpected transaction revert, missing event log). Maps to 502.
 */
public class MintingException extends ApiException {

    public MintingException(String message, Throwable cause) {
        super(message, cause);
    }

    public MintingException(String message) {
        super(message);
    }

    @Override
    public HttpStatus getStatus() {
        return HttpStatus.BAD_GATEWAY;
    }
}
