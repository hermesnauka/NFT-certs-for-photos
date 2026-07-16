"""One view class per endpoint in ../docs/sdlc/05-api-design.md, matching the controller-per-
concern layout used by the other three ports.
"""

import os

from django.http import HttpResponse
from rest_framework.parsers import FormParser, MultiPartParser
from rest_framework.response import Response
from rest_framework.views import APIView

from api import i18n, registry
from api.dto import build_etherscan_url, build_opensea_url, build_rarible_url, to_dto
from api.errors import (
    CertificateNotFoundException,
    EmptyFileException,
    MalformedIdentityException,
    UnsupportedFileTypeException,
    UnsupportedLanguageException,
    ValidationException,
)
from api.services import pdf as pdf_service
from api.services.hashing import sha256_hex
from api.services.watermark import watermark
from api.upload_store import StoredUpload, upload_store
from api.wallet_validation import DID_PATTERN, WALLET_ADDRESS_PATTERN

SUPPORTED_EXTENSIONS = {
    ".jpg": "image/jpeg",
    ".jpeg": "image/jpeg",
    ".png": "image/png",
    ".webp": "image/webp",
}


def _require_string(value, field):
    if not value:
        raise ValidationException(f"{field}: must not be blank")
    return value


def _parse_token_id(raw):
    try:
        return int(raw)
    except (TypeError, ValueError):
        raise CertificateNotFoundException(-1)


class UploadView(APIView):
    parser_classes = [MultiPartParser, FormParser]

    def post(self, request):
        file = request.FILES.get("file")
        if file is None or file.size == 0:
            raise EmptyFileException()

        _, extension = os.path.splitext(file.name)
        content_type = SUPPORTED_EXTENSIONS.get(extension.lower())
        if content_type is None:
            raise UnsupportedFileTypeException(extension.lstrip("."))

        original_content = file.read()
        sha256_hash = sha256_hex(original_content)
        # The artist DID isn't known at upload time (supplied later on POST /api/artworks), so an
        # empty marker is used here — mirrors the other ports' UploadController.
        watermarked_content = watermark(original_content, content_type, sha256_hash, "")

        file_id = upload_store.store(StoredUpload(file.name, content_type, sha256_hash, watermarked_content))

        return Response({
            "fileId": file_id,
            "originalFilename": file.name,
            "sha256Hash": sha256_hash,
            "sizeBytes": len(watermarked_content),
        })


class ArtworkListView(APIView):
    def post(self, request):
        body = request.data
        file_id = _require_string(body.get("fileId"), "fileId")
        title = _require_string(body.get("title"), "title")
        royalty_bps = body.get("royaltyPercentageBps")
        if royalty_bps is None:
            raise ValidationException("royaltyPercentageBps: must not be null")
        artist_wallet_address = _require_string(body.get("artistWalletAddress"), "artistWalletAddress")

        upload = upload_store.get(file_id)
        artwork = registry.certificate_service.create_artwork(
            upload, title, body.get("description") or "", body.get("medium") or "",
            body.get("yearCreated"), royalty_bps, artist_wallet_address, body.get("artistDid") or "",
        )

        return Response({
            "artworkId": str(artwork.id),
            "sha256Hash": artwork.sha256_hash,
            "imageIpfsUri": artwork.image_ipfs_uri,
            "metadataIpfsUri": artwork.metadata_ipfs_uri,
            "status": artwork.status,
        }, status=201)


class ArtworkMintView(APIView):
    def post(self, request, artwork_id):
        recipient_address = _require_string(request.data.get("recipientAddress"), "recipientAddress")
        certificate = registry.certificate_service.mint_certificate(artwork_id, recipient_address)
        return Response(to_dto(certificate))


class CertificateDetailView(APIView):
    def get(self, request, token_id_raw):
        certificate = registry.certificate_service.get_certificate(_parse_token_id(token_id_raw))
        return Response(to_dto(certificate))


class CertificatePdfView(APIView):
    def get(self, request, token_id_raw):
        token_id = _parse_token_id(token_id_raw)
        certificate = registry.certificate_service.get_certificate(token_id)
        pdf_bytes = pdf_service.generate(
            certificate, build_etherscan_url(certificate), build_opensea_url(certificate),
            build_rarible_url(certificate),
        )
        response = HttpResponse(pdf_bytes, content_type="application/pdf")
        response["Content-Disposition"] = f'attachment; filename="certificate-{token_id}.pdf"'
        return response


class ArtistDashboardView(APIView):
    def get(self, request, wallet_address):
        certificates = registry.certificate_service.get_certificates_for_wallet(wallet_address)
        return Response({
            "walletAddress": wallet_address,
            "certificates": [to_dto(c) for c in certificates],
            "totalCertificates": len(certificates),
        })


class I18nView(APIView):
    def get(self, request, lang):
        messages = i18n.for_language(lang)
        if messages is None:
            raise UnsupportedLanguageException(lang)
        return Response(messages)


class IdentityVerifyView(APIView):
    def post(self, request):
        body = request.data
        wallet_address = body.get("walletAddress")
        did = body.get("did")
        if not wallet_address or not did:
            raise MalformedIdentityException("walletAddress and did are required")
        if not WALLET_ADDRESS_PATTERN.match(wallet_address):
            raise MalformedIdentityException(f"Malformed wallet address: {wallet_address}")
        if not DID_PATTERN.match(did):
            raise MalformedIdentityException(f"Malformed DID: {did}")

        identity = registry.kyc_service.verify(wallet_address, did, body.get("email") or "")
        return Response({
            "verified": identity.verified,
            "did": identity.did,
            "walletAddress": identity.wallet_address,
        })
