"""API-visible errors and the shared error JSON contract.

One concrete exception class per error case (mirrors app01/app02/app03's Exceptions.* files) with
the HTTP status hardcoded, plus a DRF exception_handler that renders every exception — ApiException
or not — into the documented shape:
{ "timestamp", "status", "error", "message", "path" }
See ../docs/sdlc/05-api-design.md §9.
"""

import datetime
import logging

from rest_framework.response import Response

logger = logging.getLogger(__name__)

_REASON_PHRASES = {
    400: "Bad Request",
    403: "Forbidden",
    404: "Not Found",
    409: "Conflict",
    413: "Payload Too Large",
    422: "Unprocessable Entity",
    502: "Bad Gateway",
}


class ApiException(Exception):
    def __init__(self, message, status):
        super().__init__(message)
        self.message = message
        self.status = status


class ArtworkNotFoundException(ApiException):
    def __init__(self, artwork_id):
        super().__init__(f"No artwork found for artworkId: {artwork_id}", 404)


class ArtworkNotPinnedException(ApiException):
    def __init__(self, artwork_id):
        super().__init__(f"Artwork {artwork_id} is not yet PINNED and cannot be minted", 422)


class CertificateNotFoundException(ApiException):
    def __init__(self, token_id):
        super().__init__(f"No certificate found for tokenId: {token_id}", 404)


class ChainUnavailableException(ApiException):
    def __init__(self, message):
        super().__init__(message, 502)


class DuplicateContentHashException(ApiException):
    def __init__(self, content_hash_hex):
        super().__init__(f"A certificate already exists for content hash: {content_hash_hex}", 409)


class EmptyFileException(ApiException):
    def __init__(self):
        super().__init__("Uploaded file is empty", 400)


class FileNotFoundInUploadStoreException(ApiException):
    def __init__(self, file_id):
        super().__init__(f"No uploaded file found for fileId: {file_id}", 404)


class IpfsPinningException(ApiException):
    def __init__(self, message):
        super().__init__(message, 502)


class MalformedIdentityException(ApiException):
    def __init__(self, message):
        super().__init__(message, 400)


class MintingException(ApiException):
    def __init__(self, message):
        super().__init__(message, 502)


class NotVerifiedException(ApiException):
    def __init__(self, wallet_address):
        super().__init__(f"Wallet address is not KYC-verified: {wallet_address}", 403)


class RoyaltyOutOfRangeException(ApiException):
    def __init__(self, royalty_percentage_bps):
        super().__init__(
            f"royaltyPercentageBps must be between 0 and 10000, got: {royalty_percentage_bps}", 422
        )


class UnsupportedFileTypeException(ApiException):
    def __init__(self, content_type):
        super().__init__(f"Unsupported file type: {content_type}", 400)


class UnsupportedLanguageException(ApiException):
    def __init__(self, lang):
        super().__init__(f"Unsupported language: {lang} (supported: en, pl)", 404)


class ValidationException(ApiException):
    def __init__(self, message):
        super().__init__(message, 422)


def exception_handler(exc, context):
    status = exc.status if isinstance(exc, ApiException) else 500
    message = exc.message if isinstance(exc, ApiException) else str(exc)
    if status >= 500:
        logger.error("Unhandled exception for %s", context["request"].path, exc_info=exc)

    path = context["request"].path if context.get("request") is not None else ""
    body = {
        "timestamp": datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
        "status": status,
        "error": _REASON_PHRASES.get(status, "Internal Server Error"),
        "message": message,
        "path": path,
    }
    return Response(body, status=status)
