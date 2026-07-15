package com.devpowers.nftcerts.api;

import com.devpowers.nftcerts.certificate.Artwork;
import com.devpowers.nftcerts.certificate.Certificate;
import com.devpowers.nftcerts.config.ExplorerLinkProperties;
import org.springframework.stereotype.Component;

/** Maps {@link Certificate} entities to the {@link CertificateDto} API shape, deriving explorer links. */
@Component
public class CertificateDtoMapper {

    private final ExplorerLinkProperties explorerLinkProperties;

    public CertificateDtoMapper(ExplorerLinkProperties explorerLinkProperties) {
        this.explorerLinkProperties = explorerLinkProperties;
    }

    public CertificateDto toDto(Certificate certificate) {
        Artwork artwork = certificate.getArtwork();
        return new CertificateDto(
                certificate.getTokenId(),
                artwork == null ? null : artwork.getId(),
                artwork == null ? null : artwork.getTitle(),
                certificate.getContentHashHex(),
                certificate.getContractAddress(),
                certificate.getTxHash(),
                certificate.getOwnerAddress(),
                certificate.getRoyaltyPercentageBps(),
                artwork == null ? null : artwork.getImageIpfsUri(),
                artwork == null ? null : artwork.getMetadataIpfsUri(),
                buildEtherscanUrl(certificate),
                buildOpenSeaUrl(certificate),
                buildRaribleUrl(certificate));
    }

    public String buildEtherscanUrl(Certificate certificate) {
        return explorerLinkProperties.getEtherscanBaseUrl() + certificate.getTxHash();
    }

    public String buildOpenSeaUrl(Certificate certificate) {
        return explorerLinkProperties.getOpenseaBaseUrl() + certificate.getContractAddress() + "/" + certificate.getTokenId();
    }

    public String buildRaribleUrl(Certificate certificate) {
        return explorerLinkProperties.getRaribleBaseUrl() + certificate.getContractAddress() + ":" + certificate.getTokenId();
    }
}
