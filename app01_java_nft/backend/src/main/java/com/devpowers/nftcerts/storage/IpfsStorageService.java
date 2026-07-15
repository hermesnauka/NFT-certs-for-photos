package com.devpowers.nftcerts.storage;

/**
 * Abstraction over decentralized (IPFS) storage pinning. Two implementations exist:
 * {@link PinataIpfsStorageService} (real, used outside the {@code test} profile) and
 * {@link LocalStubIpfsStorageService} (local stub, wired only under the {@code test} profile).
 */
public interface IpfsStorageService {

    /** Pins raw file bytes (e.g. the source image) to IPFS. */
    PinResult pinFile(byte[] content, String filename, String contentType);

    /** Pins a JSON-serializable payload (e.g. NFT metadata) to IPFS. */
    PinResult pinJson(Object jsonPayload, String name);
}
