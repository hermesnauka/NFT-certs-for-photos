package com.devpowers.nftcerts.config;

import org.springframework.boot.context.properties.ConfigurationProperties;

/**
 * Configuration for the Web3j RPC connection and minter account used by {@code ContractService}.
 */
@ConfigurationProperties(prefix = "web3")
public class Web3Properties {

    private String rpcUrl = "http://localhost:8545";
    private String minterPrivateKey;
    private String contractAddress;

    public String getRpcUrl() {
        return rpcUrl;
    }

    public void setRpcUrl(String rpcUrl) {
        this.rpcUrl = rpcUrl;
    }

    public String getMinterPrivateKey() {
        return minterPrivateKey;
    }

    public void setMinterPrivateKey(String minterPrivateKey) {
        this.minterPrivateKey = minterPrivateKey;
    }

    public String getContractAddress() {
        return contractAddress;
    }

    public void setContractAddress(String contractAddress) {
        this.contractAddress = contractAddress;
    }
}
