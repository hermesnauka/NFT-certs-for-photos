package com.gandarych.nftcerts.config;

import org.springframework.boot.context.properties.ConfigurationProperties;

/**
 * Base URLs used to build {@code etherscanUrl}/{@code openSeaUrl}/{@code raribleUrl} links
 * returned in {@code CertificateDto}. Defaults point at local-network placeholders suitable for
 * a Hardhat node and public testnets.
 */
@ConfigurationProperties(prefix = "explorer")
public class ExplorerLinkProperties {

    private String etherscanBaseUrl = "http://localhost:8545/tx/";
    private String openseaBaseUrl = "https://testnets.opensea.io/assets/";
    private String raribleBaseUrl = "https://rarible.com/token/";

    public String getEtherscanBaseUrl() {
        return etherscanBaseUrl;
    }

    public void setEtherscanBaseUrl(String etherscanBaseUrl) {
        this.etherscanBaseUrl = etherscanBaseUrl;
    }

    public String getOpenseaBaseUrl() {
        return openseaBaseUrl;
    }

    public void setOpenseaBaseUrl(String openseaBaseUrl) {
        this.openseaBaseUrl = openseaBaseUrl;
    }

    public String getRaribleBaseUrl() {
        return raribleBaseUrl;
    }

    public void setRaribleBaseUrl(String raribleBaseUrl) {
        this.raribleBaseUrl = raribleBaseUrl;
    }
}
