package com.devpowers.nftcerts.identity;

import org.junit.jupiter.api.Test;
import org.mockito.ArgumentCaptor;

import java.util.Optional;
import java.util.UUID;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

class MockKycVerificationServiceTest {

    private final ArtistIdentityRepository repository = mock(ArtistIdentityRepository.class);
    private final MockKycVerificationService service = new MockKycVerificationService(repository);

    @Test
    void verifyAlwaysReturnsVerifiedTrueForNewWallet() {
        when(repository.findByWalletAddress("0xabc")).thenReturn(Optional.empty());
        when(repository.save(any())).thenAnswer(invocation -> invocation.getArgument(0));

        ArtistIdentity result = service.verify("0xabc", "did:key:zArtist", "artist@example.com");

        assertTrue(result.isVerified());
        assertEquals("0xabc", result.getWalletAddress());
        assertEquals("did:key:zArtist", result.getDid());
    }

    @Test
    void verifyUpsertsExistingIdentityByWalletAddress() {
        ArtistIdentity existing = new ArtistIdentity(UUID.randomUUID(), "0xdef", "did:key:zOld", "old@example.com", false, null);
        when(repository.findByWalletAddress("0xdef")).thenReturn(Optional.of(existing));
        when(repository.save(any())).thenAnswer(invocation -> invocation.getArgument(0));

        ArtistIdentity result = service.verify("0xdef", "did:key:zNew", "new@example.com");

        ArgumentCaptor<ArtistIdentity> captor = ArgumentCaptor.forClass(ArtistIdentity.class);
        verify(repository).save(captor.capture());

        assertEquals("did:key:zNew", captor.getValue().getDid());
        assertTrue(captor.getValue().isVerified());
        assertTrue(result.isVerified());
    }

    @Test
    void isVerifiedReturnsFalseForUnknownWallet() {
        when(repository.findByWalletAddress("0xunknown")).thenReturn(Optional.empty());

        assertTrue(!service.isVerified("0xunknown"));
    }
}
