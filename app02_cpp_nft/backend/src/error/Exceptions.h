#pragma once

#include "error/ApiException.h"

#include <drogon/HttpTypes.h>

#include <string>

// Concrete exception types, one per error case, each hardcoding its HTTP status per app01's
// error/*.java classes (see docs/sdlc/05-api-design.md #9 and app01's GlobalExceptionHandler).
namespace nftcerts::error {

class ArtworkNotFoundException : public ApiException {
public:
    explicit ArtworkNotFoundException(const std::string& artworkId)
        : ApiException("No artwork found for artworkId: " + artworkId, drogon::k404NotFound) {}
};

class ArtworkNotPinnedException : public ApiException {
public:
    explicit ArtworkNotPinnedException(const std::string& artworkId)
        : ApiException("Artwork " + artworkId + " is not yet PINNED and cannot be minted",
                        drogon::k422UnprocessableEntity) {}
};

class CertificateNotFoundException : public ApiException {
public:
    explicit CertificateNotFoundException(long long tokenId)
        : ApiException("No certificate found for tokenId: " + std::to_string(tokenId), drogon::k404NotFound) {}
};

class ChainUnavailableException : public ApiException {
public:
    explicit ChainUnavailableException(const std::string& message)
        : ApiException(message, drogon::k502BadGateway) {}
};

class DuplicateContentHashException : public ApiException {
public:
    explicit DuplicateContentHashException(const std::string& contentHashHex)
        : ApiException("A certificate already exists for content hash: " + contentHashHex, drogon::k409Conflict) {}
};

class EmptyFileException : public ApiException {
public:
    EmptyFileException() : ApiException("Uploaded file is empty", drogon::k400BadRequest) {}
};

class FileNotFoundInUploadStoreException : public ApiException {
public:
    explicit FileNotFoundInUploadStoreException(const std::string& fileId)
        : ApiException("No uploaded file found for fileId: " + fileId, drogon::k404NotFound) {}
};

class FileTooLargeException : public ApiException {
public:
    FileTooLargeException(size_t sizeBytes, size_t maxBytes)
        : ApiException("Uploaded file size " + std::to_string(sizeBytes) + " bytes exceeds maximum of " +
                            std::to_string(maxBytes) + " bytes",
                        drogon::k413RequestEntityTooLarge) {}
};

class IpfsPinningException : public ApiException {
public:
    explicit IpfsPinningException(const std::string& message)
        : ApiException(message, drogon::k502BadGateway) {}
};

class MalformedIdentityException : public ApiException {
public:
    explicit MalformedIdentityException(const std::string& message)
        : ApiException(message, drogon::k400BadRequest) {}
};

class MintingException : public ApiException {
public:
    explicit MintingException(const std::string& message)
        : ApiException(message, drogon::k502BadGateway) {}
};

class NotVerifiedException : public ApiException {
public:
    explicit NotVerifiedException(const std::string& walletAddress)
        : ApiException("Wallet address is not KYC-verified: " + walletAddress, drogon::k403Forbidden) {}
};

class RoyaltyOutOfRangeException : public ApiException {
public:
    explicit RoyaltyOutOfRangeException(long long royaltyPercentageBps)
        : ApiException("royaltyPercentageBps must be between 0 and 10000, got: " +
                            std::to_string(royaltyPercentageBps),
                        drogon::k422UnprocessableEntity) {}
};

class UnsupportedFileTypeException : public ApiException {
public:
    explicit UnsupportedFileTypeException(const std::string& contentType)
        : ApiException("Unsupported file type: " + contentType, drogon::k400BadRequest) {}
};

class UnsupportedLanguageException : public ApiException {
public:
    explicit UnsupportedLanguageException(const std::string& lang)
        : ApiException("Unsupported language: " + lang + " (supported: en, pl)", drogon::k404NotFound) {}
};

class ValidationException : public ApiException {
public:
    explicit ValidationException(const std::string& message)
        : ApiException(message, drogon::k422UnprocessableEntity) {}
};

}  // namespace nftcerts::error
