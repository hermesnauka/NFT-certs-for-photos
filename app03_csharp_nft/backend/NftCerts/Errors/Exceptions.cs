namespace NftCerts.Errors;

// Base for every API-visible error, carrying its HTTP status. Mirrors app01's error/*.java and
// app02's error/Exceptions.h — one concrete type per error case with the status hardcoded.
public class ApiException : Exception
{
    public int Status { get; }

    public ApiException(string message, int status) : base(message) => Status = status;
}

public class ArtworkNotFoundException(string artworkId)
    : ApiException($"No artwork found for artworkId: {artworkId}", StatusCodes.Status404NotFound);

public class ArtworkNotPinnedException(string artworkId)
    : ApiException($"Artwork {artworkId} is not yet PINNED and cannot be minted",
                    StatusCodes.Status422UnprocessableEntity);

public class CertificateNotFoundException(long tokenId)
    : ApiException($"No certificate found for tokenId: {tokenId}", StatusCodes.Status404NotFound);

public class ChainUnavailableException(string message) : ApiException(message, StatusCodes.Status502BadGateway);

public class DuplicateContentHashException(string contentHashHex)
    : ApiException($"A certificate already exists for content hash: {contentHashHex}",
                    StatusCodes.Status409Conflict);

public class EmptyFileException() : ApiException("Uploaded file is empty", StatusCodes.Status400BadRequest);

public class FileNotFoundInUploadStoreException(string fileId)
    : ApiException($"No uploaded file found for fileId: {fileId}", StatusCodes.Status404NotFound);

public class IpfsPinningException(string message) : ApiException(message, StatusCodes.Status502BadGateway);

public class MalformedIdentityException(string message) : ApiException(message, StatusCodes.Status400BadRequest);

public class MintingException(string message) : ApiException(message, StatusCodes.Status502BadGateway);

public class NotVerifiedException(string walletAddress)
    : ApiException($"Wallet address is not KYC-verified: {walletAddress}", StatusCodes.Status403Forbidden);

public class RoyaltyOutOfRangeException(long royaltyPercentageBps)
    : ApiException($"royaltyPercentageBps must be between 0 and 10000, got: {royaltyPercentageBps}",
                    StatusCodes.Status422UnprocessableEntity);

public class UnsupportedFileTypeException(string contentType)
    : ApiException($"Unsupported file type: {contentType}", StatusCodes.Status400BadRequest);

public class UnsupportedLanguageException(string lang)
    : ApiException($"Unsupported language: {lang} (supported: en, pl)", StatusCodes.Status404NotFound);

public class ValidationException(string message)
    : ApiException(message, StatusCodes.Status422UnprocessableEntity);
