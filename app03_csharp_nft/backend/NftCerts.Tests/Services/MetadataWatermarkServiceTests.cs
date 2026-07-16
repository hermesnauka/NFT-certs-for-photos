using System.Text;
using System.Text.Json;
using Microsoft.Extensions.Logging.Abstractions;
using NftCerts.Watermark;
using SixLabors.ImageSharp;
using SixLabors.ImageSharp.Formats.Jpeg;
using SixLabors.ImageSharp.Metadata.Profiles.Exif;
using SixLabors.ImageSharp.PixelFormats;

namespace NftCerts.Tests.Services;

public class MetadataWatermarkServiceTests
{
    private readonly MetadataWatermarkService _service = new(NullLogger<MetadataWatermarkService>.Instance);

    private static byte[] GenerateJpeg()
    {
        using var image = new Image<Rgba32>(4, 4);
        using var output = new MemoryStream();
        image.Save(output, new JpegEncoder());
        return output.ToArray();
    }

    [Fact]
    public void Watermark_JpegInput_EmbedsHashAndDidRoundTrippably()
    {
        byte[] original = GenerateJpeg();
        const string hash = "9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a0";
        const string did = "did:key:z6MkhaXgBZDvotDkL5257faiztiGiC2QtKLGpbnnEGta2doK";

        byte[] watermarked = _service.Watermark(original, "image/jpeg", hash, did);

        using var reloaded = Image.Load(watermarked);
        ExifProfile? exif = reloaded.Metadata.ExifProfile;
        Assert.NotNull(exif);
        IExifValue<string>? descriptionValue = exif!.GetValue(ExifTag.ImageDescription);
        Assert.NotNull(descriptionValue);
        var payload = JsonDocument.Parse(descriptionValue!.Value!).RootElement;
        Assert.Equal(hash, payload.GetProperty("contentHash").GetString());
        Assert.Equal(did, payload.GetProperty("artistDid").GetString());
    }

    [Fact]
    public void Watermark_UnsupportedContentType_PassesThroughUnchanged()
    {
        byte[] original = Encoding.UTF8.GetBytes("not actually a png, but content type says so");

        byte[] result = _service.Watermark(original, "image/png", "somehash", "did:key:zabc");

        Assert.Equal(original, result);
    }

    [Fact]
    public void Watermark_CorruptJpegInput_FallsBackToOriginalBytes()
    {
        byte[] corrupt = [0xFF, 0xD8, 0xFF, 0x00, 0x01, 0x02, 0x03];

        byte[] result = _service.Watermark(corrupt, "image/jpeg", "somehash", "did:key:zabc");

        Assert.Equal(corrupt, result);
    }
}
