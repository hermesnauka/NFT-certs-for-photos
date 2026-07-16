using System.Net;

namespace NftCerts.Tests.TestSupport;

// Minimal IHttpClientFactory fake so PinataIpfsStorageService's outbound calls can be tested
// without a live Pinata endpoint (NFR-4).
public class FakeHttpClientFactory(Func<HttpRequestMessage, HttpResponseMessage> respond) : IHttpClientFactory
{
    public HttpRequestMessage? LastRequest { get; private set; }

    public HttpClient CreateClient(string name) => new(new CapturingHandler(this, respond));

    // PinataIpfsStorageService calls the synchronous HttpClient.Send, which requires the handler
    // to override the synchronous Send (not just SendAsync).
    private class CapturingHandler(FakeHttpClientFactory owner, Func<HttpRequestMessage, HttpResponseMessage> respond)
        : HttpMessageHandler
    {
        protected override HttpResponseMessage Send(HttpRequestMessage request, CancellationToken cancellationToken)
        {
            owner.LastRequest = request;
            return respond(request);
        }

        protected override Task<HttpResponseMessage> SendAsync(HttpRequestMessage request,
                                                                 CancellationToken cancellationToken) =>
            Task.FromResult(Send(request, cancellationToken));
    }

    public static HttpResponseMessage JsonResponse(HttpStatusCode status, string json) => new(status)
    {
        Content = new StringContent(json),
    };
}
