package com.devpowers.nftcerts.error;

import org.springframework.http.HttpStatus;

/** Thrown when the configured Ethereum RPC endpoint is unreachable. Maps to 502. */
public class ChainUnavailableException extends ApiException {

    public ChainUnavailableException(String message, Throwable cause) {
        super(message, cause);
    }

    public ChainUnavailableException(String message) {
        super(message);
    }

    @Override
    public HttpStatus getStatus() {
        return HttpStatus.BAD_GATEWAY;
    }
}
