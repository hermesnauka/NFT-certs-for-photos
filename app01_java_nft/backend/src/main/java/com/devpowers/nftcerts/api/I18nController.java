package com.devpowers.nftcerts.api;

import com.devpowers.nftcerts.error.UnsupportedLanguageException;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RestController;

import java.util.Collections;
import java.util.Enumeration;
import java.util.LinkedHashMap;
import java.util.Locale;
import java.util.Map;
import java.util.ResourceBundle;
import java.util.Set;

/**
 * GET /api/i18n/{lang} — flattens the {@code messages_en}/{@code messages_pl} resource bundles
 * into a JSON map, backing the frontend's Polish/English UI language switcher.
 */
@RestController
public class I18nController {

    private static final Set<String> SUPPORTED_LANGUAGES = Set.of("en", "pl");

    @GetMapping("/api/i18n/{lang}")
    public ResponseEntity<Map<String, String>> getMessages(@PathVariable String lang) {
        if (!SUPPORTED_LANGUAGES.contains(lang)) {
            throw new UnsupportedLanguageException(lang);
        }

        ResourceBundle bundle = ResourceBundle.getBundle("messages", Locale.forLanguageTag(lang));
        Map<String, String> messages = new LinkedHashMap<>();
        Enumeration<String> keys = bundle.getKeys();
        for (String key : Collections.list(keys)) {
            messages.put(key, bundle.getString(key));
        }
        return ResponseEntity.ok(messages);
    }
}
