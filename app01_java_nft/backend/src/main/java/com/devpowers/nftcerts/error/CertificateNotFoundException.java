package com.devpowers.nftcerts.error;

import org.springframework.http.HttpStatus;

/** Thrown when a referenced {@code tokenId} does not exist. Maps to 404. */
public class CertificateNotFoundException extends ApiException {

    public CertificateNotFoundException(Long tokenId) {
        super("No certificate found for tokenId: " + tokenId);
    }

    @Override
    public HttpStatus getStatus() {
        return HttpStatus.NOT_FOUND;
    }
}
