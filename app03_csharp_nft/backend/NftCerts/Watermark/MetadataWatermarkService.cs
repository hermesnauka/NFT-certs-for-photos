using System.Text.Json;
using SixLabors.ImageSharp;
using SixLabors.ImageSharp.Formats.Jpeg;
using SixLabors.ImageSharp.Metadata.Profiles.Exif;

namespace NftCerts.Watermark;

// Embeds a JSON payload referencing the content hash and artist DID into the JPEG's
// Exif.Image.ImageDescription tag via ImageSharp. Metadata-based (not steganographic) by design —
// see the ADR in docs/sdlc/10-glossary-and-decisions.md. Unsupported content types and unreadable
// images gracefully pass the original bytes through, matching app01/app02 behavior.
public class MetadataWatermarkService(ILogger<MetadataWatermarkService> logger)
{
    private const string JpegContentType = "image/jpeg";

    public byte[] Watermark(byte[] originalContent, string contentType, string contentHash, string artistDid)
    {
        if (!string.Equals(contentType, JpegContentType, StringComparison.OrdinalIgnoreCase))
        {
            logger.LogInformation(
                "Skipping EXIF watermark embedding for unsupported content type '{ContentType}': only {Supported} is supported",
                contentType, JpegContentType);
            return originalContent;
        }

        try
        {
            string payloadJson = JsonSerializer.Serialize(new { contentHash, artistDid });

            using var image = Image.Load(originalContent);
            image.Metadata.ExifProfile ??= new ExifProfile();
            image.Metadata.ExifProfile.SetValue(ExifTag.ImageDescription, payloadJson);

            using var output = new MemoryStream();
            image.Save(output, new JpegEncoder());
            return output.ToArray();
        }
        catch (Exception exception) when (exception is not OutOfMemoryException)
        {
            logger.LogWarning(exception,
                "Failed to embed EXIF watermark, returning original file content unchanged");
            return originalContent;
        }
    }
}
