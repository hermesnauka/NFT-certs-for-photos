package com.devpowers.nftcerts.error;

import org.springframework.http.HttpStatus;

import java.util.UUID;

/** Thrown when minting is attempted before an artwork has finished IPFS pinning. Maps to 422. */
public class ArtworkNotPinnedException extends ApiException {

    public ArtworkNotPinnedException(UUID artworkId) {
        super("Artwork " + artworkId + " is not yet PINNED and cannot be minted");
    }

    @Override
    public HttpStatus getStatus() {
        return HttpStatus.UNPROCESSABLE_ENTITY;
    }
}
