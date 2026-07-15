package com.devpowers.nftcerts.error;

import org.springframework.http.HttpStatus;

/** Thrown for any {@code lang} other than {@code en}/{@code pl} on the i18n endpoint. Maps to 404. */
public class UnsupportedLanguageException extends ApiException {

    public UnsupportedLanguageException(String lang) {
        super("Unsupported language: " + lang + " (supported: en, pl)");
    }

    @Override
    public HttpStatus getStatus() {
        return HttpStatus.NOT_FOUND;
    }
}
