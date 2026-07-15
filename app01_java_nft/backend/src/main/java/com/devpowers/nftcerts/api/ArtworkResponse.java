package com.devpowers.nftcerts.api;

import com.devpowers.nftcerts.certificate.ArtworkStatus;

import java.util.UUID;

public record ArtworkResponse(
        UUID artworkId,
        String sha256Hash,
        String imageIpfsUri,
        String metadataIpfsUri,
        ArtworkStatus status
) {
}
