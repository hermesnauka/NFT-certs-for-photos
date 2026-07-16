"""Orchestrates the artwork -> IPFS pinning -> on-chain minting -> Certificate persistence
pipeline. Mirrors the other ports' CertificateService.
"""

import datetime
import uuid

from django.core.exceptions import ValidationError

from api.errors import (
    ArtworkNotFoundException,
    ArtworkNotPinnedException,
    CertificateNotFoundException,
    DuplicateContentHashException,
    NotVerifiedException,
    RoyaltyOutOfRangeException,
)
from api.models import Artwork, ArtworkStatus, Certificate


def _now_iso8601():
    return datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


class CertificateService:
    def __init__(self, storage, contract_service, kyc_service):
        self.storage = storage
        self.contract_service = contract_service
        self.kyc_service = kyc_service

    def create_artwork(self, upload, title, description, medium, year_created, royalty_percentage_bps,
                        artist_wallet_address, artist_did):
        if royalty_percentage_bps is None or royalty_percentage_bps < 0 or royalty_percentage_bps > 10000:
            raise RoyaltyOutOfRangeException(royalty_percentage_bps if royalty_percentage_bps is not None else -1)

        artwork = Artwork(
            id=uuid.uuid4(),
            file_id=uuid.uuid4(),
            original_filename=upload.original_filename,
            sha256_hash=upload.sha256_hash,
            title=title,
            description=description,
            medium=medium,
            year_created=year_created,
            royalty_percentage_bps=royalty_percentage_bps,
            artist_wallet_address=artist_wallet_address,
            artist_did=artist_did,
            status=ArtworkStatus.UPLOADED,
            created_at=_now_iso8601(),
        )

        image_pin = self.storage.pin_file(upload.content, upload.original_filename, upload.content_type)
        artwork.image_ipfs_uri = image_pin.uri

        metadata = {
            "title": artwork.title,
            "description": artwork.description,
            "medium": artwork.medium,
            "yearCreated": artwork.year_created,
            "artistDid": artwork.artist_did,
            "sha256Hash": artwork.sha256_hash,
            "image": image_pin.uri,
        }
        metadata_pin = self.storage.pin_json(metadata, f"{artwork.id}-metadata")
        artwork.metadata_ipfs_uri = metadata_pin.uri

        artwork.status = ArtworkStatus.PINNED
        artwork.save()
        return artwork

    def mint_certificate(self, artwork_id, recipient_address):
        try:
            artwork = Artwork.objects.get(id=artwork_id)
        except (Artwork.DoesNotExist, ValueError, ValidationError):
            raise ArtworkNotFoundException(artwork_id)

        if artwork.status != ArtworkStatus.PINNED:
            raise ArtworkNotPinnedException(artwork_id)
        if not self.kyc_service.is_verified(artwork.artist_wallet_address):
            raise NotVerifiedException(artwork.artist_wallet_address)
        if Certificate.objects.filter(content_hash_hex=artwork.sha256_hash).exists():
            raise DuplicateContentHashException(artwork.sha256_hash)

        mint_result = self.contract_service.mint_certificate(
            recipient_address, artwork.metadata_ipfs_uri, artwork.sha256_hash,
            artwork.artist_wallet_address, artwork.royalty_percentage_bps,
        )

        certificate = Certificate(
            token_id=mint_result.token_id,
            artwork=artwork,
            content_hash_hex=artwork.sha256_hash,
            contract_address=mint_result.contract_address,
            tx_hash=mint_result.tx_hash,
            owner_address=recipient_address,
            royalty_percentage_bps=artwork.royalty_percentage_bps,
            royalty_recipient=artwork.artist_wallet_address,
            minted_at=_now_iso8601(),
        )
        artwork.status = ArtworkStatus.MINTED
        artwork.save()
        certificate.save()
        return certificate

    def get_certificate(self, token_id):
        try:
            return Certificate.objects.select_related("artwork").get(token_id=token_id)
        except Certificate.DoesNotExist:
            raise CertificateNotFoundException(token_id)

    def get_certificates_for_wallet(self, wallet_address):
        return list(Certificate.objects.select_related("artwork").filter(owner_address=wallet_address))
