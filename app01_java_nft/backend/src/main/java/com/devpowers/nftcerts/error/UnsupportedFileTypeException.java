package com.devpowers.nftcerts.error;

import org.springframework.http.HttpStatus;

/** Thrown when an uploaded file's content type is not supported. Maps to 400. */
public class UnsupportedFileTypeException extends ApiException {

    public UnsupportedFileTypeException(String contentType) {
        super("Unsupported file type: " + contentType);
    }

    @Override
    public HttpStatus getStatus() {
        return HttpStatus.BAD_REQUEST;
    }
}
