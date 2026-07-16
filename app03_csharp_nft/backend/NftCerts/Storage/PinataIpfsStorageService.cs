using System.Net.Http.Headers;
using System.Text;
using System.Text.Json;
using NftCerts.Config;
using NftCerts.Errors;

namespace NftCerts.Storage;

// Real Pinata pinning via pinFileToIPFS / pinJSONToIPFS. Mirrors app01's PinataIpfsStorageService.
public class PinataIpfsStorageService(PinataProperties properties, IHttpClientFactory httpClientFactory)
    : IIpfsStorageService
{
    private const string PinFilePath = "/pinning/pinFileToIPFS";
    private const string PinJsonPath = "/pinning/pinJSONToIPFS";

    public PinResult PinFile(byte[] content, string filename, string contentType)
    {
        using var form = new MultipartFormDataContent();
        var fileContent = new ByteArrayContent(content);
        fileContent.Headers.ContentType =
            new MediaTypeHeaderValue(contentType.Length > 0 ? contentType : "application/octet-stream");
        form.Add(fileContent, "file", filename);
        return Send(PinFilePath, form, "file");
    }

    public PinResult PinJson(JsonElement jsonPayload, string name)
    {
        string wrapper = JsonSerializer.Serialize(new
        {
            pinataMetadata = new { name },
            pinataContent = jsonPayload,
        });
        var body = new StringContent(wrapper, Encoding.UTF8, "application/json");
        return Send(PinJsonPath, body, "JSON");
    }

    private PinResult Send(string path, HttpContent content, string what)
    {
        var request = new HttpRequestMessage(HttpMethod.Post, properties.BaseUrl.TrimEnd('/') + path)
        {
            Content = content,
        };
        if (properties.HasJwt)
        {
            request.Headers.Authorization = new AuthenticationHeaderValue("Bearer", properties.Jwt);
        }
        else if (properties.HasApiKeyPair)
        {
            request.Headers.Add("pinata_api_key", properties.ApiKey);
            request.Headers.Add("pinata_secret_api_key", properties.ApiSecret);
        }
        else
        {
            throw new IpfsPinningException(
                "No Pinata credentials configured (PINATA_JWT or PINATA_API_KEY/PINATA_API_SECRET)");
        }

        HttpResponseMessage response;
        string responseBody;
        try
        {
            var client = httpClientFactory.CreateClient("pinata");
            response = client.Send(request, HttpCompletionOption.ResponseContentRead);
            responseBody = response.Content.ReadAsStringAsync().GetAwaiter().GetResult();
        }
        catch (Exception exception) when (exception is HttpRequestException or TaskCanceledException)
        {
            throw new IpfsPinningException($"Failed to pin {what} to IPFS via Pinata: request failed");
        }

        if (!response.IsSuccessStatusCode)
        {
            throw new IpfsPinningException(
                $"Failed to pin {what} to IPFS via Pinata: HTTP {(int)response.StatusCode}");
        }
        return PinResult.Of(ExtractCid(responseBody));
    }

    internal static string ExtractCid(string responseBody)
    {
        JsonElement root;
        try
        {
            root = JsonDocument.Parse(responseBody).RootElement;
        }
        catch (JsonException)
        {
            throw new IpfsPinningException($"Failed to parse Pinata response: {responseBody}");
        }
        if (!root.TryGetProperty("IpfsHash", out var hash) || hash.GetString() is not { Length: > 0 } cid)
        {
            throw new IpfsPinningException($"Pinata response did not contain an IpfsHash: {responseBody}");
        }
        return cid;
    }
}
