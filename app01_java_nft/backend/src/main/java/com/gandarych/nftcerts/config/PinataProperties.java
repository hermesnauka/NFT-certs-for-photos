package com.gandarych.nftcerts.config;

import org.springframework.boot.context.properties.ConfigurationProperties;

/**
 * Configuration for Pinata IPFS pinning authentication.
 * Prefers {@code jwt}; falls back to the {@code apiKey}/{@code apiSecret} pair if unset.
 */
@ConfigurationProperties(prefix = "pinata")
public class PinataProperties {

    private String jwt;
    private String apiKey;
    private String apiSecret;
    private String baseUrl = "https://api.pinata.cloud";

    public String getJwt() {
        return jwt;
    }

    public void setJwt(String jwt) {
        this.jwt = jwt;
    }

    public String getApiKey() {
        return apiKey;
    }

    public void setApiKey(String apiKey) {
        this.apiKey = apiKey;
    }

    public String getApiSecret() {
        return apiSecret;
    }

    public void setApiSecret(String apiSecret) {
        this.apiSecret = apiSecret;
    }

    public String getBaseUrl() {
        return baseUrl;
    }

    public void setBaseUrl(String baseUrl) {
        this.baseUrl = baseUrl;
    }

    public boolean hasJwt() {
        return jwt != null && !jwt.isBlank();
    }

    public boolean hasApiKeyPair() {
        return apiKey != null && !apiKey.isBlank() && apiSecret != null && !apiSecret.isBlank();
    }
}
