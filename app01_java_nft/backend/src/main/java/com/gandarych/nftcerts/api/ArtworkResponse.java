package com.gandarych.nftcerts.api;

import com.gandarych.nftcerts.certificate.ArtworkStatus;

import java.util.UUID;

public record ArtworkResponse(
        UUID artworkId,
        String sha256Hash,
        String imageIpfsUri,
        String metadataIpfsUri,
        ArtworkStatus status
) {
}
