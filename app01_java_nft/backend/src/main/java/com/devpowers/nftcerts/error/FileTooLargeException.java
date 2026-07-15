package com.devpowers.nftcerts.error;

import org.springframework.http.HttpStatus;

/** Thrown when an uploaded file exceeds the configured maximum size. Maps to 413. */
public class FileTooLargeException extends ApiException {

    public FileTooLargeException(long sizeBytes, long maxBytes) {
        super("Uploaded file size " + sizeBytes + " bytes exceeds maximum of " + maxBytes + " bytes");
    }

    @Override
    public HttpStatus getStatus() {
        return HttpStatus.PAYLOAD_TOO_LARGE;
    }
}
