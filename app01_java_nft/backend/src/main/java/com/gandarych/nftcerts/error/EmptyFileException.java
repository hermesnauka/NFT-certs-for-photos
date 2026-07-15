package com.gandarych.nftcerts.error;

import org.springframework.http.HttpStatus;

/** Thrown when an uploaded file is empty. Maps to 400. */
public class EmptyFileException extends ApiException {

    public EmptyFileException() {
        super("Uploaded file is empty");
    }

    @Override
    public HttpStatus getStatus() {
        return HttpStatus.BAD_REQUEST;
    }
}
