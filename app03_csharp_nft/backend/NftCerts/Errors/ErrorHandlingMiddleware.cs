using System.Text.Json;

namespace NftCerts.Errors;

// Maps exceptions to the shared error JSON contract used by app01/app02:
// { "error", "message", "path", "status", "timestamp" }.
public class ErrorHandlingMiddleware(RequestDelegate next, ILogger<ErrorHandlingMiddleware> logger)
{
    public async Task InvokeAsync(HttpContext context)
    {
        try
        {
            await next(context);
        }
        catch (Exception exception)
        {
            int status = exception is ApiException apiException
                ? apiException.Status
                : StatusCodes.Status500InternalServerError;
            if (status >= 500)
            {
                logger.LogError(exception, "Unhandled exception for {Path}", context.Request.Path);
            }

            context.Response.StatusCode = status;
            context.Response.ContentType = "application/json";
            var body = new Dictionary<string, object>
            {
                ["error"] = ReasonPhrase(status),
                ["message"] = exception.Message,
                ["path"] = context.Request.Path.ToString(),
                ["status"] = status,
                ["timestamp"] = DateTime.UtcNow.ToString("yyyy-MM-dd'T'HH:mm:ss'Z'"),
            };
            await context.Response.WriteAsync(JsonSerializer.Serialize(body));
        }
    }

    private static string ReasonPhrase(int status) => status switch
    {
        400 => "Bad Request",
        403 => "Forbidden",
        404 => "Not Found",
        409 => "Conflict",
        413 => "Payload Too Large",
        422 => "Unprocessable Entity",
        502 => "Bad Gateway",
        _ => "Internal Server Error",
    };
}
