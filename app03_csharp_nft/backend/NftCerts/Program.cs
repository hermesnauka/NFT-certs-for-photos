using Microsoft.EntityFrameworkCore;
using NftCerts.Api;
using NftCerts.Blockchain;
using NftCerts.Certificates;
using NftCerts.Config;
using NftCerts.Db;
using NftCerts.Errors;
using NftCerts.Hashing;
using NftCerts.Identity;
using NftCerts.Pdf;
using NftCerts.Storage;
using NftCerts.Watermark;

var builder = WebApplication.CreateBuilder(args);

AppConfig config = AppConfig.LoadFromEnv();
if (!builder.Environment.IsEnvironment("Testing"))
{
    AppConfig.ValidateStartupConfig(config);
}

builder.Services.AddSingleton(config);
builder.Services.AddSingleton(config.Pinata);
builder.Services.AddSingleton(config.Web3);
builder.Services.AddSingleton(config.Explorer);
builder.Services.AddSingleton(config.Storage);

builder.Services.AddDbContext<NftCertsDbContext>(options => options.UseSqlite($"Data Source={config.DbPath}"));

builder.Services.AddHttpClient("pinata");

builder.Services.AddSingleton<Sha256HashingService>();
builder.Services.AddSingleton<MetadataWatermarkService>();
builder.Services.AddSingleton<UploadStore>();
builder.Services.AddSingleton<CertificateDtoMapper>();
builder.Services.AddSingleton<CertificatePdfService>();
builder.Services.AddScoped<ContractService>();
builder.Services.AddScoped<CertificateService>();
builder.Services.AddScoped<IKycVerificationService, MockKycVerificationService>();

if (config.Storage.IsMock)
{
    builder.Services.AddSingleton<IIpfsStorageService, LocalStubIpfsStorageService>();
}
else
{
    builder.Services.AddScoped<IIpfsStorageService, PinataIpfsStorageService>();
}

builder.Services.AddControllers();
builder.Services.AddOpenApi();

if (!builder.Environment.IsEnvironment("Testing"))
{
    builder.WebHost.UseUrls($"http://0.0.0.0:{config.ServerPort}");
}

var app = builder.Build();

app.Logger.LogInformation("Active storage provider: {Provider}", config.Storage.Provider);

using (var scope = app.Services.CreateScope())
{
    var db = scope.ServiceProvider.GetRequiredService<NftCertsDbContext>();
    db.Database.EnsureCreated();
}

if (app.Environment.IsDevelopment())
{
    app.MapOpenApi();
}

app.UseMiddleware<ErrorHandlingMiddleware>();
app.MapControllers();

app.Run();

public partial class Program;
