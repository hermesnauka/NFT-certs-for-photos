package com.devpowers.nftcerts.error;

import org.springframework.http.HttpStatus;

/** Thrown when a wallet address or DID fails basic format validation. Maps to 400. */
public class MalformedIdentityException extends ApiException {

    public MalformedIdentityException(String message) {
        super(message);
    }

    @Override
    public HttpStatus getStatus() {
        return HttpStatus.BAD_REQUEST;
    }
}
