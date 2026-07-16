using Microsoft.AspNetCore.Hosting;
using Microsoft.AspNetCore.Mvc.Testing;
using Microsoft.Data.Sqlite;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.DependencyInjection.Extensions;
using NftCerts.Blockchain;
using NftCerts.Db;
using NftCerts.Storage;

namespace NftCerts.Tests.TestSupport;

// Real HTTP pipeline (production routes, middleware, DI graph), swapping only the two external
// dependencies that would otherwise need a live chain/Pinata: IIpfsStorageService and
// ContractService. A single open in-memory SQLite connection is shared for the factory's lifetime
// so state persists across requests within one test, matching the per-process H2/SQLite behavior
// of the production app.
public class CustomWebApplicationFactory : WebApplicationFactory<Program>
{
    private readonly SqliteConnection _connection = new("Data Source=:memory:");

    public FakeContractService FakeContractService { get; } = new();

    protected override void ConfigureWebHost(IWebHostBuilder builder)
    {
        _connection.Open();
        builder.UseEnvironment("Testing");
        builder.ConfigureServices(services =>
        {
            services.RemoveAll<DbContextOptions<NftCertsDbContext>>();
            services.AddDbContext<NftCertsDbContext>(options => options.UseSqlite(_connection));

            services.RemoveAll<IIpfsStorageService>();
            services.AddSingleton<IIpfsStorageService, LocalStubIpfsStorageService>();

            services.RemoveAll<ContractService>();
            services.AddSingleton<ContractService>(FakeContractService);
        });
    }

    protected override void Dispose(bool disposing)
    {
        base.Dispose(disposing);
        if (disposing)
        {
            _connection.Dispose();
        }
    }
}
