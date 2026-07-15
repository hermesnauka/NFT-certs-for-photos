package com.gandarych.nftcerts.config;

import org.springframework.boot.context.properties.ConfigurationProperties;

/**
 * Selects which {@code IpfsStorageService} implementation is wired: {@code pinata} (default, real
 * IPFS pinning) or {@code mock} (local-disk fake, no credentials needed). See
 * {@code PinataIpfsStorageService} and {@code LocalStubIpfsStorageService}.
 */
@ConfigurationProperties(prefix = "app.storage")
public class StorageProperties {

    private String provider = "pinata";

    public String getProvider() {
        return provider;
    }

    public void setProvider(String provider) {
        this.provider = provider;
    }

    public boolean isMock() {
        return "mock".equalsIgnoreCase(provider);
    }

    public boolean isPinata() {
        return "pinata".equalsIgnoreCase(provider);
    }
}
