import pytest

from api.services.identity import MockKycVerificationService


@pytest.mark.django_db
def test_is_verified_unknown_wallet_returns_false():
    service = MockKycVerificationService()
    assert service.is_verified("0xUnknown") is False


@pytest.mark.django_db
def test_verify_always_approves_and_persists_identity():
    service = MockKycVerificationService()

    identity = service.verify("0xWallet1", "did:key:zabc", "artist@example.com")

    assert identity.verified is True
    assert identity.did == "did:key:zabc"
    assert service.is_verified("0xWallet1") is True


@pytest.mark.django_db
def test_verify_called_twice_for_same_wallet_updates_existing_record():
    service = MockKycVerificationService()

    service.verify("0xWallet1", "did:key:zabc", "old@example.com")
    updated = service.verify("0xWallet1", "did:key:znew", "new@example.com")

    assert updated.did == "did:key:znew"
    from api.models import ArtistIdentity
    assert ArtistIdentity.objects.count() == 1
