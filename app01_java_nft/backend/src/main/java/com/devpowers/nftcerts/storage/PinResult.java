package com.devpowers.nftcerts.storage;

/** Result of pinning content (a file or a JSON document) to IPFS: the resulting CID and its URI. */
public record PinResult(String cid, String uri) {

    public static PinResult of(String cid) {
        return new PinResult(cid, "ipfs://" + cid);
    }
}
