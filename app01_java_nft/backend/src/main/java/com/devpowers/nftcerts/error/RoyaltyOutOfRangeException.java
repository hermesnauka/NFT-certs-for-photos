package com.devpowers.nftcerts.error;

import org.springframework.http.HttpStatus;

/** Thrown when {@code royaltyPercentageBps} is outside the valid 0-10000 range. Maps to 422. */
public class RoyaltyOutOfRangeException extends ApiException {

    public RoyaltyOutOfRangeException(Integer royaltyPercentageBps) {
        super("royaltyPercentageBps must be between 0 and 10000, got: " + royaltyPercentageBps);
    }

    @Override
    public HttpStatus getStatus() {
        return HttpStatus.UNPROCESSABLE_ENTITY;
    }
}
