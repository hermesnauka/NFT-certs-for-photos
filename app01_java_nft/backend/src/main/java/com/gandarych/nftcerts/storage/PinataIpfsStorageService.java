package com.gandarych.nftcerts.storage;

import com.gandarych.nftcerts.config.PinataProperties;
import com.gandarych.nftcerts.error.IpfsPinningException;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import org.springframework.context.annotation.Profile;
import org.springframework.core.io.ByteArrayResource;
import org.springframework.http.HttpHeaders;
import org.springframework.http.MediaType;
import org.springframework.stereotype.Service;
import org.springframework.util.LinkedMultiValueMap;
import org.springframework.util.MultiValueMap;
import org.springframework.web.client.RestClient;
import org.springframework.web.client.RestClientException;

import java.util.LinkedHashMap;
import java.util.Map;

/**
 * Real {@link IpfsStorageService} implementation backed by the Pinata pinning API. Used as the
 * primary bean in every profile except {@code test}.
 */
@Service
@Profile("!test")
public class PinataIpfsStorageService implements IpfsStorageService {

    private static final String PIN_FILE_PATH = "/pinning/pinFileToIPFS";
    private static final String PIN_JSON_PATH = "/pinning/pinJSONToIPFS";

    private final RestClient restClient;
    private final PinataProperties pinataProperties;
    private final ObjectMapper objectMapper;

    public PinataIpfsStorageService(RestClient restClient, PinataProperties pinataProperties, ObjectMapper objectMapper) {
        this.restClient = restClient;
        this.pinataProperties = pinataProperties;
        this.objectMapper = objectMapper;
    }

    @Override
    public PinResult pinFile(byte[] content, String filename, String contentType) {
        MultiValueMap<String, Object> body = new LinkedMultiValueMap<>();
        ByteArrayResource resource = new ByteArrayResource(content) {
            @Override
            public String getFilename() {
                return filename;
            }
        };
        body.add("file", resource);

        try {
            String response = applyAuth(restClient.post().uri(pinataProperties.getBaseUrl() + PIN_FILE_PATH))
                    .contentType(MediaType.MULTIPART_FORM_DATA)
                    .body(body)
                    .retrieve()
                    .body(String.class);
            return PinResult.of(extractCid(response));
        } catch (RestClientException e) {
            throw new IpfsPinningException("Failed to pin file to IPFS via Pinata: " + e.getMessage(), e);
        }
    }

    @Override
    public PinResult pinJson(Object jsonPayload, String name) {
        Map<String, Object> wrapper = new LinkedHashMap<>();
        wrapper.put("pinataMetadata", Map.of("name", name));
        wrapper.put("pinataContent", jsonPayload);

        try {
            String response = applyAuth(restClient.post().uri(pinataProperties.getBaseUrl() + PIN_JSON_PATH))
                    .contentType(MediaType.APPLICATION_JSON)
                    .body(wrapper)
                    .retrieve()
                    .body(String.class);
            return PinResult.of(extractCid(response));
        } catch (RestClientException e) {
            throw new IpfsPinningException("Failed to pin JSON to IPFS via Pinata: " + e.getMessage(), e);
        }
    }

    private RestClient.RequestBodySpec applyAuth(RestClient.RequestBodySpec spec) {
        if (pinataProperties.hasJwt()) {
            return spec.header(HttpHeaders.AUTHORIZATION, "Bearer " + pinataProperties.getJwt());
        } else if (pinataProperties.hasApiKeyPair()) {
            return spec.header("pinata_api_key", pinataProperties.getApiKey())
                    .header("pinata_secret_api_key", pinataProperties.getApiSecret());
        } else {
            throw new IpfsPinningException("No Pinata credentials configured (PINATA_JWT or PINATA_API_KEY/PINATA_API_SECRET)");
        }
    }

    private String extractCid(String responseBody) {
        try {
            JsonNode node = objectMapper.readTree(responseBody);
            JsonNode cidNode = node.get("IpfsHash");
            if (cidNode == null || cidNode.asText().isBlank()) {
                throw new IpfsPinningException("Pinata response did not contain an IpfsHash: " + responseBody);
            }
            return cidNode.asText();
        } catch (com.fasterxml.jackson.core.JsonProcessingException e) {
            throw new IpfsPinningException("Failed to parse Pinata response: " + responseBody, e);
        }
    }
}
