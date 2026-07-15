package com.gandarych.nftcerts.error;

import org.springframework.http.HttpStatus;

import java.util.UUID;

/** Thrown when a referenced {@code artworkId} does not exist. Maps to 404. */
public class ArtworkNotFoundException extends ApiException {

    public ArtworkNotFoundException(UUID artworkId) {
        super("No artwork found for artworkId: " + artworkId);
    }

    @Override
    public HttpStatus getStatus() {
        return HttpStatus.NOT_FOUND;
    }
}
