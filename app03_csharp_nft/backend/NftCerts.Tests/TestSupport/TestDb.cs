using Microsoft.Data.Sqlite;
using Microsoft.EntityFrameworkCore;
using NftCerts.Db;

namespace NftCerts.Tests.TestSupport;

// A fresh, schema-created NftCertsDbContext backed by a private in-memory SQLite database, for
// tests that need real EF Core query/persistence behavior without a live/shared database file.
public sealed class TestDb : IDisposable
{
    private readonly SqliteConnection _connection = new("Data Source=:memory:");

    public NftCertsDbContext Context { get; }

    public TestDb()
    {
        _connection.Open();
        var options = new DbContextOptionsBuilder<NftCertsDbContext>().UseSqlite(_connection).Options;
        Context = new NftCertsDbContext(options);
        Context.Database.EnsureCreated();
    }

    public void Dispose()
    {
        Context.Dispose();
        _connection.Dispose();
    }
}
