package com.gandarych.nftcerts.config;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
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

    private static final Logger log = LoggerFactory.getLogger(StartupConfigurationValidator.class);

    private final PinataProperties pinataProperties;
    private final Web3Properties web3Properties;
    private final StorageProperties storageProperties;

    public StartupConfigurationValidator(PinataProperties pinataProperties, Web3Properties web3Properties,
                                          StorageProperties storageProperties) {
        this.pinataProperties = pinataProperties;
        this.web3Properties = web3Properties;
        this.storageProperties = storageProperties;
    }

    @Override
    public void run(ApplicationArguments args) {
        if (!storageProperties.isMock() && !storageProperties.isPinata()) {
            throw new IllegalStateException(
                    "Invalid app.storage.provider '" + storageProperties.getProvider() + "': must be 'pinata' or 'mock'");
        }
        log.info("Storage provider: {}", storageProperties.getProvider());

        if (storageProperties.isPinata() && !pinataProperties.hasJwt() && !pinataProperties.hasApiKeyPair()) {
            throw new IllegalStateException(
                    "Missing Pinata credentials: set PINATA_JWT, or both PINATA_API_KEY and PINATA_API_SECRET "
                            + "(or set app.storage.provider=mock to run without Pinata)");
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
