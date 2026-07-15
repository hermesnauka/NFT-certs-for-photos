package com.gandarych.nftcerts.error;

import org.springframework.http.HttpStatus;

import java.util.UUID;

/** Thrown when a referenced {@code fileId} is not present in the upload store. Maps to 404. */
public class FileNotFoundInUploadStoreException extends ApiException {

    public FileNotFoundInUploadStoreException(UUID fileId) {
        super("No uploaded file found for fileId: " + fileId);
    }

    @Override
    public HttpStatus getStatus() {
        return HttpStatus.NOT_FOUND;
    }
}
