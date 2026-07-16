using Microsoft.EntityFrameworkCore;
using NftCerts.Certificates;
using NftCerts.Identity;

namespace NftCerts.Db;

// EF Core over SQLite (app01 used JPA/H2, app02 raw SQLite). Table/entity set mirrors both.
public class NftCertsDbContext(DbContextOptions<NftCertsDbContext> options) : DbContext(options)
{
    public DbSet<Artwork> Artworks => Set<Artwork>();
    public DbSet<Certificate> Certificates => Set<Certificate>();
    public DbSet<ArtistIdentity> ArtistIdentities => Set<ArtistIdentity>();

    protected override void OnModelCreating(ModelBuilder modelBuilder)
    {
        modelBuilder.Entity<Certificate>()
            .HasOne(c => c.Artwork)
            .WithMany()
            .HasForeignKey(c => c.ArtworkId);
        modelBuilder.Entity<Certificate>().Navigation(c => c.Artwork).AutoInclude();
        modelBuilder.Entity<Artwork>().Property(a => a.Status).HasConversion<string>();
    }
}
