package com.devpowers.nftcerts.error;

import org.springframework.http.HttpStatus;

/** Thrown when pinning content to IPFS via Pinata fails (non-2xx response or network error). Maps to 502. */
public class IpfsPinningException extends ApiException {

    public IpfsPinningException(String message) {
        super(message);
    }

    public IpfsPinningException(String message, Throwable cause) {
        super(message, cause);
    }

    @Override
    public HttpStatus getStatus() {
        return HttpStatus.BAD_GATEWAY;
    }
}
