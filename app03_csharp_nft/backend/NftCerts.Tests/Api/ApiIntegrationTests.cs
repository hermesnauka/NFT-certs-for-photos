using System.Net;
using System.Net.Http.Headers;
using System.Net.Http.Json;
using System.Text;
using System.Text.Json;
using NftCerts.Tests.TestSupport;

namespace NftCerts.Tests.Api;

// Full HTTP pipeline tests against the real production routes/DI graph (only IIpfsStorageService
// and ContractService are swapped for offline fakes — see CustomWebApplicationFactory), covering
// the happy path end to end plus the documented error-JSON contract for each reachable 4xx.
public class ApiIntegrationTests(CustomWebApplicationFactory factory) : IClassFixture<CustomWebApplicationFactory>
{
    private readonly HttpClient _client = factory.CreateClient();

    private static string RandomWallet() =>
        "0x" + (Guid.NewGuid().ToString("N") + Guid.NewGuid().ToString("N"))[..40];

    private static string RandomDid() => $"did:key:z{Guid.NewGuid():N}";

    private static MultipartFormDataContent JpegUpload(byte[] bytes, string filename = "photo.jpg")
    {
        var content = new MultipartFormDataContent();
        var fileContent = new ByteArrayContent(bytes);
        fileContent.Headers.ContentType = new MediaTypeHeaderValue("image/jpeg");
        content.Add(fileContent, "file", filename);
        return content;
    }

    private static byte[] UniqueJpegBytes([System.Runtime.CompilerServices.CallerMemberName] string seed = "") =>
        Encoding.UTF8.GetBytes($"fake-jpeg-bytes-{seed}-{Guid.NewGuid()}");

    [Fact]
    public async Task FullHappyPath_UploadThroughDashboardAndPdf()
    {
        string wallet = RandomWallet();
        string did = RandomDid();

        using HttpResponseMessage uploadResponse = await _client.PostAsync("/api/uploads", JpegUpload(UniqueJpegBytes()));
        Assert.Equal(HttpStatusCode.OK, uploadResponse.StatusCode);
        JsonElement upload = await uploadResponse.Content.ReadFromJsonAsync<JsonElement>();
        string fileId = upload.GetProperty("fileId").GetString()!;
        Assert.False(string.IsNullOrEmpty(upload.GetProperty("sha256Hash").GetString()));

        using HttpResponseMessage verifyResponse = await _client.PostAsJsonAsync("/api/identity/verify", new
        {
            walletAddress = wallet, did, email = "artist@example.com",
        });
        Assert.Equal(HttpStatusCode.OK, verifyResponse.StatusCode);
        JsonElement verify = await verifyResponse.Content.ReadFromJsonAsync<JsonElement>();
        Assert.True(verify.GetProperty("verified").GetBoolean());

        using HttpResponseMessage artworkResponse = await _client.PostAsJsonAsync("/api/artworks", new
        {
            fileId, title = "Lighthouse at Dusk", description = "A long exposure coastal photograph.",
            medium = "Archival pigment print", yearCreated = 2024, royaltyPercentageBps = 750,
            artistWalletAddress = wallet, artistDid = did,
        });
        Assert.Equal(HttpStatusCode.Created, artworkResponse.StatusCode);
        JsonElement artwork = await artworkResponse.Content.ReadFromJsonAsync<JsonElement>();
        string artworkId = artwork.GetProperty("artworkId").GetString()!;
        Assert.Equal("PINNED", artwork.GetProperty("status").GetString());
        Assert.StartsWith("ipfs://", artwork.GetProperty("imageIpfsUri").GetString());

        using HttpResponseMessage mintResponse =
            await _client.PostAsJsonAsync($"/api/artworks/{artworkId}/mint", new { recipientAddress = wallet });
        Assert.Equal(HttpStatusCode.OK, mintResponse.StatusCode);
        JsonElement certificate = await mintResponse.Content.ReadFromJsonAsync<JsonElement>();
        long tokenId = certificate.GetProperty("tokenId").GetInt64();
        Assert.Equal(wallet, certificate.GetProperty("ownerAddress").GetString());
        Assert.Equal(750, certificate.GetProperty("royaltyPercentageBps").GetInt32());

        using HttpResponseMessage getResponse = await _client.GetAsync($"/api/certificates/{tokenId}");
        Assert.Equal(HttpStatusCode.OK, getResponse.StatusCode);
        JsonElement fetched = await getResponse.Content.ReadFromJsonAsync<JsonElement>();
        Assert.Equal(artworkId, fetched.GetProperty("artworkId").GetString());

        using HttpResponseMessage pdfResponse = await _client.GetAsync($"/api/certificates/{tokenId}/pdf");
        Assert.Equal(HttpStatusCode.OK, pdfResponse.StatusCode);
        Assert.Equal("application/pdf", pdfResponse.Content.Headers.ContentType?.MediaType);
        byte[] pdfBytes = await pdfResponse.Content.ReadAsByteArrayAsync();
        Assert.NotEmpty(pdfBytes);

        using HttpResponseMessage dashboardResponse = await _client.GetAsync($"/api/artists/{wallet}/dashboard");
        Assert.Equal(HttpStatusCode.OK, dashboardResponse.StatusCode);
        JsonElement dashboard = await dashboardResponse.Content.ReadFromJsonAsync<JsonElement>();
        Assert.Equal(1, dashboard.GetProperty("totalCertificates").GetInt32());
        Assert.Equal(tokenId, dashboard.GetProperty("certificates")[0].GetProperty("tokenId").GetInt64());
    }

    [Fact]
    public async Task Upload_EmptyFile_Returns400()
    {
        using HttpResponseMessage response = await _client.PostAsync("/api/uploads", JpegUpload([]));
        Assert.Equal(HttpStatusCode.BadRequest, response.StatusCode);
    }

    [Fact]
    public async Task Upload_UnsupportedFileType_Returns400()
    {
        using HttpResponseMessage response =
            await _client.PostAsync("/api/uploads", JpegUpload(UniqueJpegBytes(), "notes.txt"));
        Assert.Equal(HttpStatusCode.BadRequest, response.StatusCode);
    }

    [Fact]
    public async Task CreateArtwork_UnknownFileId_Returns404()
    {
        using HttpResponseMessage response = await _client.PostAsJsonAsync("/api/artworks", new
        {
            fileId = Guid.NewGuid().ToString(), title = "T", royaltyPercentageBps = 100,
            artistWalletAddress = RandomWallet(),
        });
        Assert.Equal(HttpStatusCode.NotFound, response.StatusCode);
    }

    [Fact]
    public async Task CreateArtwork_RoyaltyOutOfRange_Returns422()
    {
        using HttpResponseMessage uploadResponse = await _client.PostAsync("/api/uploads", JpegUpload(UniqueJpegBytes()));
        JsonElement upload = await uploadResponse.Content.ReadFromJsonAsync<JsonElement>();

        using HttpResponseMessage response = await _client.PostAsJsonAsync("/api/artworks", new
        {
            fileId = upload.GetProperty("fileId").GetString(), title = "T", royaltyPercentageBps = 10001,
            artistWalletAddress = RandomWallet(),
        });
        Assert.Equal(HttpStatusCode.UnprocessableEntity, response.StatusCode);
    }

    [Fact]
    public async Task Mint_UnknownArtworkId_Returns404()
    {
        using HttpResponseMessage response = await _client.PostAsJsonAsync(
            $"/api/artworks/{Guid.NewGuid()}/mint", new { recipientAddress = RandomWallet() });
        Assert.Equal(HttpStatusCode.NotFound, response.StatusCode);
    }

    [Fact]
    public async Task Mint_UnverifiedArtist_Returns403()
    {
        string wallet = RandomWallet();
        using HttpResponseMessage uploadResponse = await _client.PostAsync("/api/uploads", JpegUpload(UniqueJpegBytes()));
        JsonElement upload = await uploadResponse.Content.ReadFromJsonAsync<JsonElement>();

        using HttpResponseMessage artworkResponse = await _client.PostAsJsonAsync("/api/artworks", new
        {
            fileId = upload.GetProperty("fileId").GetString(), title = "T", royaltyPercentageBps = 100,
            artistWalletAddress = wallet,
        });
        JsonElement artwork = await artworkResponse.Content.ReadFromJsonAsync<JsonElement>();

        using HttpResponseMessage mintResponse = await _client.PostAsJsonAsync(
            $"/api/artworks/{artwork.GetProperty("artworkId").GetString()}/mint", new { recipientAddress = wallet });
        Assert.Equal(HttpStatusCode.Forbidden, mintResponse.StatusCode);
    }

    [Fact]
    public async Task GetCertificate_UnknownTokenId_Returns404()
    {
        using HttpResponseMessage response = await _client.GetAsync("/api/certificates/999999");
        Assert.Equal(HttpStatusCode.NotFound, response.StatusCode);
    }

    [Fact]
    public async Task GetCertificatePdf_UnknownTokenId_Returns404()
    {
        using HttpResponseMessage response = await _client.GetAsync("/api/certificates/999999/pdf");
        Assert.Equal(HttpStatusCode.NotFound, response.StatusCode);
    }

    [Fact]
    public async Task Dashboard_UnknownWallet_ReturnsEmptyNotError()
    {
        using HttpResponseMessage response = await _client.GetAsync($"/api/artists/{RandomWallet()}/dashboard");
        Assert.Equal(HttpStatusCode.OK, response.StatusCode);
        JsonElement dashboard = await response.Content.ReadFromJsonAsync<JsonElement>();
        Assert.Equal(0, dashboard.GetProperty("totalCertificates").GetInt32());
        Assert.Empty(dashboard.GetProperty("certificates").EnumerateArray());
    }

    [Theory]
    [InlineData("en")]
    [InlineData("pl")]
    public async Task I18n_SupportedLanguage_ReturnsMessageMap(string lang)
    {
        using HttpResponseMessage response = await _client.GetAsync($"/api/i18n/{lang}");
        Assert.Equal(HttpStatusCode.OK, response.StatusCode);
        JsonElement messages = await response.Content.ReadFromJsonAsync<JsonElement>();
        Assert.True(messages.EnumerateObject().Any());
    }

    [Fact]
    public async Task I18n_UnsupportedLanguage_Returns404()
    {
        using HttpResponseMessage response = await _client.GetAsync("/api/i18n/de");
        Assert.Equal(HttpStatusCode.NotFound, response.StatusCode);
    }

    [Fact]
    public async Task IdentityVerify_MalformedWalletAddress_Returns400()
    {
        using HttpResponseMessage response = await _client.PostAsJsonAsync("/api/identity/verify", new
        {
            walletAddress = "not-a-wallet", did = RandomDid(), email = "a@b.com",
        });
        Assert.Equal(HttpStatusCode.BadRequest, response.StatusCode);
    }

    [Fact]
    public async Task IdentityVerify_MalformedDid_Returns400()
    {
        using HttpResponseMessage response = await _client.PostAsJsonAsync("/api/identity/verify", new
        {
            walletAddress = RandomWallet(), did = "not-a-did", email = "a@b.com",
        });
        Assert.Equal(HttpStatusCode.BadRequest, response.StatusCode);
    }

    [Fact]
    public async Task ErrorResponse_MatchesDocumentedJsonContract()
    {
        using HttpResponseMessage response = await _client.GetAsync("/api/certificates/999999");
        JsonElement body = await response.Content.ReadFromJsonAsync<JsonElement>();

        Assert.True(body.TryGetProperty("timestamp", out _));
        Assert.Equal(404, body.GetProperty("status").GetInt32());
        Assert.Equal("Not Found", body.GetProperty("error").GetString());
        Assert.True(body.TryGetProperty("message", out _));
        Assert.Equal("/api/certificates/999999", body.GetProperty("path").GetString());
    }
}
