from types import SimpleNamespace

import pytest

from api import registry
from api.errors import (
    ArtworkNotFoundException,
    ArtworkNotPinnedException,
    CertificateNotFoundException,
    DuplicateContentHashException,
    NotVerifiedException,
    RoyaltyOutOfRangeException,
)
from api.models import Artwork, ArtworkStatus, Certificate
from api.upload_store import StoredUpload

pytestmark = pytest.mark.django_db


def _sample_upload(sha256="aa11bb22cc33dd44ee55ff66aa11bb22cc33dd44ee55ff66aa11bb22cc33dd4"):
    return StoredUpload("photo.jpg", "image/jpeg", sha256, b"\x01\x02\x03\x04")


@pytest.mark.parametrize("royalty_bps", [-1, 10001, None])
def test_create_artwork_royalty_out_of_range_raises(royalty_bps):
    with pytest.raises(RoyaltyOutOfRangeException):
        registry.certificate_service.create_artwork(
            _sample_upload(), "Title", "desc", "medium", 2024, royalty_bps, "0xWallet", "did:key:z",
        )


def test_create_artwork_valid_input_pins_and_persists_as_pinned():
    artwork = registry.certificate_service.create_artwork(
        _sample_upload(), "Title", "desc", "medium", 2024, 500, "0xWallet", "did:key:z",
    )

    assert artwork.status == ArtworkStatus.PINNED
    assert artwork.image_ipfs_uri.startswith("ipfs://stub-")
    assert artwork.metadata_ipfs_uri.startswith("ipfs://stub-")
    assert Artwork.objects.count() == 1


def test_mint_certificate_unknown_artwork_id_raises_not_found():
    with pytest.raises(ArtworkNotFoundException):
        registry.certificate_service.mint_certificate("00000000-0000-0000-0000-000000000000", "0xRecipient")


def test_mint_certificate_artwork_not_yet_pinned_raises_unprocessable():
    artwork = Artwork.objects.create(
        id="11111111-1111-1111-1111-111111111111", file_id="22222222-2222-2222-2222-222222222222",
        original_filename="photo.jpg", sha256_hash="hash1", royalty_percentage_bps=100,
        artist_wallet_address="0xWallet", status=ArtworkStatus.UPLOADED, created_at="2026-01-01T00:00:00Z",
    )

    with pytest.raises(ArtworkNotPinnedException):
        registry.certificate_service.mint_certificate(str(artwork.id), "0xRecipient")


def test_mint_certificate_artist_not_verified_raises_forbidden():
    artwork = registry.certificate_service.create_artwork(
        _sample_upload(), "Title", "desc", "medium", 2024, 500, "0xUnverifiedWallet", "did:key:z",
    )

    with pytest.raises(NotVerifiedException):
        registry.certificate_service.mint_certificate(str(artwork.id), "0xRecipient")


def test_mint_certificate_duplicate_content_hash_raises_conflict(monkeypatch):
    # Artwork.sha256_hash carries a DB-level unique constraint, so two Artwork rows can never
    # actually share one hash — this state is unreachable via the real (SQLite-backed) app both
    # through the API and through direct persistence. Certificate.objects.filter(...).exists() is
    # monkeypatched here purely to isolate and assert CertificateService.mint_certificate's own
    # duplicate-hash guard, independent of that outer constraint.
    registry.kyc_service.verify("0xWallet", "did:key:z", "artist@example.com")
    artwork = registry.certificate_service.create_artwork(
        _sample_upload(), "Title", "desc", "medium", 2024, 500, "0xWallet", "did:key:z",
    )
    monkeypatch.setattr(Certificate.objects, "filter", lambda **kwargs: SimpleNamespace(exists=lambda: True))

    with pytest.raises(DuplicateContentHashException):
        registry.certificate_service.mint_certificate(str(artwork.id), "0xRecipient")


def test_mint_certificate_happy_path_persists_certificate_and_marks_artwork_minted():
    registry.kyc_service.verify("0xWallet", "did:key:z", "artist@example.com")
    artwork = registry.certificate_service.create_artwork(
        _sample_upload(), "Title", "desc", "medium", 2024, 500, "0xWallet", "did:key:z",
    )

    certificate = registry.certificate_service.mint_certificate(str(artwork.id), "0xRecipient")

    assert certificate.token_id == 1
    assert certificate.owner_address == "0xRecipient"
    assert certificate.content_hash_hex == artwork.sha256_hash
    artwork.refresh_from_db()
    assert artwork.status == ArtworkStatus.MINTED


def test_get_certificate_unknown_token_id_raises_not_found():
    with pytest.raises(CertificateNotFoundException):
        registry.certificate_service.get_certificate(999)


def test_get_certificates_for_wallet_returns_only_that_wallets_certificates():
    registry.kyc_service.verify("0xWallet", "did:key:z", "artist@example.com")
    artwork = registry.certificate_service.create_artwork(
        _sample_upload(), "Title", "desc", "medium", 2024, 500, "0xWallet", "did:key:z",
    )
    registry.certificate_service.mint_certificate(str(artwork.id), "0xRecipient")

    for_recipient = registry.certificate_service.get_certificates_for_wallet("0xRecipient")
    for_other = registry.certificate_service.get_certificates_for_wallet("0xSomeoneElse")

    assert len(for_recipient) == 1
    assert len(for_other) == 0
