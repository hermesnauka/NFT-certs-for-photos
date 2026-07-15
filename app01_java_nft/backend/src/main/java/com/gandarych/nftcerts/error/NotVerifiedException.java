package com.gandarych.nftcerts.error;

import org.springframework.http.HttpStatus;

/** Thrown when minting is attempted by a wallet without a verified {@code ArtistIdentity}. Maps to 403. */
public class NotVerifiedException extends ApiException {

    public NotVerifiedException(String walletAddress) {
        super("Wallet address is not KYC-verified: " + walletAddress);
    }

    @Override
    public HttpStatus getStatus() {
        return HttpStatus.FORBIDDEN;
    }
}
