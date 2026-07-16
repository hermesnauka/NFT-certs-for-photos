using System.Net;
using System.Text.Json;
using NftCerts.Config;
using NftCerts.Errors;
using NftCerts.Storage;
using NftCerts.Tests.TestSupport;

namespace NftCerts.Tests.Services;

public class PinataIpfsStorageServiceTests
{
    [Fact]
    public void PinFile_WithJwt_SendsBearerAuthAndReturnsCid()
    {
        var factory = new FakeHttpClientFactory(request =>
        {
            Assert.Equal("Bearer", request.Headers.Authorization?.Scheme);
            Assert.Equal("test-jwt", request.Headers.Authorization?.Parameter);
            return FakeHttpClientFactory.JsonResponse(HttpStatusCode.OK, """{"IpfsHash":"bafyImageCid"}""");
        });
        var service = new PinataIpfsStorageService(new PinataProperties { Jwt = "test-jwt" }, factory);

        PinResult result = service.PinFile([1, 2, 3], "photo.jpg", "image/jpeg");

        Assert.Equal("bafyImageCid", result.Cid);
        Assert.Equal("ipfs://bafyImageCid", result.Uri);
    }

    [Fact]
    public void PinJson_WithApiKeyPair_SendsKeySecretHeadersAndReturnsCid()
    {
        var factory = new FakeHttpClientFactory(request =>
        {
            Assert.Equal("key123", Assert.Single(request.Headers.GetValues("pinata_api_key")));
            Assert.Equal("secret456", Assert.Single(request.Headers.GetValues("pinata_secret_api_key")));
            return FakeHttpClientFactory.JsonResponse(HttpStatusCode.OK, """{"IpfsHash":"bafyMetadataCid"}""");
        });
        var service = new PinataIpfsStorageService(
            new PinataProperties { ApiKey = "key123", ApiSecret = "secret456" }, factory);

        PinResult result = service.PinJson(JsonSerializer.SerializeToElement(new { a = 1 }), "metadata");

        Assert.Equal("bafyMetadataCid", result.Cid);
    }

    [Fact]
    public void PinFile_NoCredentialsConfigured_ThrowsWithoutSendingRequest()
    {
        var factory = new FakeHttpClientFactory(_ => throw new InvalidOperationException("should not be called"));
        var service = new PinataIpfsStorageService(new PinataProperties(), factory);

        Assert.Throws<IpfsPinningException>(() => service.PinFile([1], "a.jpg", "image/jpeg"));
    }

    [Fact]
    public void PinFile_NonSuccessStatusCode_ThrowsIpfsPinningException()
    {
        var factory = new FakeHttpClientFactory(_ =>
            FakeHttpClientFactory.JsonResponse(HttpStatusCode.InternalServerError, "oops"));
        var service = new PinataIpfsStorageService(new PinataProperties { Jwt = "jwt" }, factory);

        Assert.Throws<IpfsPinningException>(() => service.PinFile([1], "a.jpg", "image/jpeg"));
    }

    [Fact]
    public void PinFile_ResponseMissingIpfsHash_ThrowsIpfsPinningException()
    {
        var factory = new FakeHttpClientFactory(_ =>
            FakeHttpClientFactory.JsonResponse(HttpStatusCode.OK, """{"unexpected":"shape"}"""));
        var service = new PinataIpfsStorageService(new PinataProperties { Jwt = "jwt" }, factory);

        Assert.Throws<IpfsPinningException>(() => service.PinFile([1], "a.jpg", "image/jpeg"));
    }
}
