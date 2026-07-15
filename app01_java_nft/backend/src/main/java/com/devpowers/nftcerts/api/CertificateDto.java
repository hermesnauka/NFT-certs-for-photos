package com.devpowers.nftcerts.api;

import java.util.UUID;

public record CertificateDto(
        Long tokenId,
        UUID artworkId,
        String title,
        String contentHashHex,
        String contractAddress,
        String txHash,
        String ownerAddress,
        Integer royaltyPercentageBps,
        String imageIpfsUri,
        String metadataIpfsUri,
        String etherscanUrl,
        String openSeaUrl,
        String raribleUrl
) {
}
