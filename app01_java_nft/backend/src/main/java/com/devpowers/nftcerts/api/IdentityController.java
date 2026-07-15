package com.devpowers.nftcerts.api;

import com.devpowers.nftcerts.error.MalformedIdentityException;
import com.devpowers.nftcerts.identity.ArtistIdentity;
import com.devpowers.nftcerts.identity.KycVerificationService;
import jakarta.validation.Valid;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RestController;

import java.util.regex.Pattern;

/** POST /api/identity/verify — artist identity verification (DID / KYC) flow. */
@RestController
public class IdentityController {

    private static final Pattern WALLET_ADDRESS_PATTERN = Pattern.compile("^0x[a-fA-F0-9]{40}$");
    private static final Pattern DID_PATTERN = Pattern.compile("^did:[a-zA-Z0-9]+:.+$");

    private final KycVerificationService kycVerificationService;

    public IdentityController(KycVerificationService kycVerificationService) {
        this.kycVerificationService = kycVerificationService;
    }

    @PostMapping("/api/identity/verify")
    public ResponseEntity<IdentityVerifyResponse> verify(@Valid @RequestBody IdentityVerifyRequest request) {
        if (!WALLET_ADDRESS_PATTERN.matcher(request.walletAddress()).matches()) {
            throw new MalformedIdentityException("Malformed wallet address: " + request.walletAddress());
        }
        if (!DID_PATTERN.matcher(request.did()).matches()) {
            throw new MalformedIdentityException("Malformed DID: " + request.did());
        }

        ArtistIdentity identity = kycVerificationService.verify(request.walletAddress(), request.did(), request.email());

        return ResponseEntity.ok(new IdentityVerifyResponse(identity.isVerified(), identity.getDid(), identity.getWalletAddress()));
    }
}
