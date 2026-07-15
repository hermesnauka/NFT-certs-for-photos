package com.gandarych.nftcerts.blockchain;

import com.gandarych.nftcerts.config.Web3Properties;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Profile;
import org.web3j.protocol.Web3j;
import org.web3j.protocol.http.HttpService;

/**
 * Wires the {@link Web3j} client used by {@link ContractService} against the configured RPC
 * endpoint. Not created under the {@code test} profile — unit tests supply a Mockito mock of
 * {@link Web3j} directly to {@link ContractService} instead of hitting a real/local node.
 */
@Configuration
@Profile("!test")
public class Web3jConfig {

    @Bean
    public Web3j web3j(Web3Properties web3Properties) {
        return Web3j.build(new HttpService(web3Properties.getRpcUrl()));
    }
}
