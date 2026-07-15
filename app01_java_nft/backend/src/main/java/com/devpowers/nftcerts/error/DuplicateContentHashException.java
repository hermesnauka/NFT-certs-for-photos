package com.devpowers.nftcerts.error;

import org.springframework.http.HttpStatus;

/**
 * Thrown when a content hash has already been certified — either detected by the backend before
 * sending a transaction, or surfaced by mapping the on-chain
 * {@code "PhotoCertificate: duplicate content hash"} revert reason. Maps to 409.
 */
public class DuplicateContentHashException extends ApiException {

    public DuplicateContentHashException(String contentHashHex) {
        super("A certificate already exists for content hash: " + contentHashHex);
    }

    @Override
    public HttpStatus getStatus() {
        return HttpStatus.CONFLICT;
    }
}
