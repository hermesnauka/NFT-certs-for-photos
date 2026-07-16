import uuid

from django.db import models


class ArtworkStatus(models.TextChoices):
    UPLOADED = "UPLOADED"
    PINNED = "PINNED"
    MINTED = "MINTED"


class Artwork(models.Model):
    """An uploaded photograph after hashing/watermarking, before or after minting.

    Mirrors the Artwork entity in ../docs/sdlc/04-data-model.md.
    """

    id = models.UUIDField(primary_key=True, default=uuid.uuid4, editable=False)
    file_id = models.UUIDField()
    original_filename = models.CharField(max_length=255)
    sha256_hash = models.CharField(max_length=64, unique=True)
    title = models.CharField(max_length=255, default="")
    description = models.TextField(default="")
    medium = models.CharField(max_length=255, default="")
    year_created = models.IntegerField(null=True, blank=True)
    royalty_percentage_bps = models.IntegerField()
    artist_wallet_address = models.CharField(max_length=42)
    artist_did = models.CharField(max_length=255, default="")
    image_ipfs_uri = models.CharField(max_length=255, default="")
    metadata_ipfs_uri = models.CharField(max_length=255, default="")
    status = models.CharField(max_length=16, choices=ArtworkStatus.choices, default=ArtworkStatus.UPLOADED)
    created_at = models.CharField(max_length=32)  # ISO-8601 instant, matches the other ports' string timestamps


class Certificate(models.Model):
    """A successfully minted on-chain NFT; a read-optimized mirror of on-chain facts.

    Mirrors the Certificate entity in ../docs/sdlc/04-data-model.md. tokenId comes from the chain,
    not auto-generated.
    """

    token_id = models.BigIntegerField(primary_key=True)
    artwork = models.OneToOneField(Artwork, on_delete=models.CASCADE, related_name="certificate")
    content_hash_hex = models.CharField(max_length=64)
    contract_address = models.CharField(max_length=42)
    tx_hash = models.CharField(max_length=255)
    owner_address = models.CharField(max_length=42)
    royalty_percentage_bps = models.IntegerField()
    royalty_recipient = models.CharField(max_length=42)
    minted_at = models.CharField(max_length=32)


class ArtistIdentity(models.Model):
    """Result of the mock KYC/DID verification flow. Mirrors ../docs/sdlc/04-data-model.md."""

    wallet_address = models.CharField(max_length=42, primary_key=True)
    did = models.CharField(max_length=255, default="")
    email = models.CharField(max_length=255, default="")
    verified = models.BooleanField(default=False)
    verified_at = models.CharField(max_length=32, default="")
