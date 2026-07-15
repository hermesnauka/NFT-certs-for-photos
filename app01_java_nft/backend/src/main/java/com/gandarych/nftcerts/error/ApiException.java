package com.gandarych.nftcerts.error;

import org.springframework.http.HttpStatus;

/**
 * Base type for all domain exceptions that map to a specific HTTP status via
 * {@link GlobalExceptionHandler}. Concrete subclasses exist per error case so control flow never
 * relies on generic/raw exceptions.
 */
public abstract class ApiException extends RuntimeException {

    protected ApiException(String message) {
        super(message);
    }

    protected ApiException(String message, Throwable cause) {
        super(message, cause);
    }

    public abstract HttpStatus getStatus();
}
