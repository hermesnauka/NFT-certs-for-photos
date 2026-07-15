package com.devpowers.nftcerts.config;

import org.springframework.boot.ApplicationArguments;
import org.springframework.boot.ApplicationRunner;
import org.springframework.context.annotation.Profile;
import org.springframework.stereotype.Component;

/**
 * Fails fast at startup (under any profile other than {@code test}) if required external-service
 * configuration is missing, rather than letting misconfiguration surface as an NPE deep inside a
 * request handler.
 */
@Component
@Profile("!test")
public class StartupConfigurationValidator implements ApplicationRunner {

    private final PinataProperties pinataProperties;
    private final Web3Properties web3Properties;

    public StartupConfigurationValidator(PinataProperties pinataProperties, Web3Properties web3Properties) {
        this.pinataProperties = pinataProperties;
        this.web3Properties = web3Properties;
    }

    @Override
    public void run(ApplicationArguments args) {
        if (!pinataProperties.hasJwt() && !pinataProperties.hasApiKeyPair()) {
            throw new IllegalStateException(
                    "Missing Pinata credentials: set PINATA_JWT, or both PINATA_API_KEY and PINATA_API_SECRET");
        }
        if (isBlank(web3Properties.getContractAddress())) {
            throw new IllegalStateException("Missing required environment variable: NFT_CONTRACT_ADDRESS");
        }
        if (isBlank(web3Properties.getMinterPrivateKey())) {
            throw new IllegalStateException("Missing required environment variable: MINTER_PRIVATE_KEY");
        }
    }

    private static boolean isBlank(String value) {
        return value == null || value.isBlank();
    }
}
