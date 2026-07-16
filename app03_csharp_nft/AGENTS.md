# app03_csharp_nft — C#/.NET Port

.NET 10 (C#) port of the reference implementation in `../app01_java_nft`. Full product spec: root `../requirements.md` (`../requirements_pl.md` for Polish); `docs/sdlc/` in this folder for architecture, API, smart-contract design, threat model, and test plan.

## Layout
- `contracts/` — Hardhat + Solidity (ERC-721 + EIP-2981 + Pausable/Burnable), same design as app01. Local dev chain only.
- `backend/` — ASP.NET Core Web API (.NET 10), EF Core + SQLite. SHA-256 hashing (`System.Security.Cryptography`), EXIF watermarking (SixLabors.ImageSharp, pinned to the last Apache-2.0 release — see below), Pinata IPFS pinning + local stub, mock KYC, Nethereum for chain calls (typed `FunctionMessage`/`IEventDTO` classes hand-written per call rather than a full ABI-codegen wrapper — mirrors app01's "explicit ABI encoding" rationale), PDF certificates (QuestPDF), EN/PL i18n message maps.
- `frontend/` — Next.js 14 + TypeScript + Tailwind + wagmi/viem (same stack as app01).
- `docs/sdlc/` — SDLC documentation set (numbered 01-10).

## Build & test
- Contracts: `cd contracts && npm install && npx hardhat test`
- Backend (requires the .NET 10 SDK; if `dotnet` isn't on `PATH`, it may be at `~/.dotnet/dotnet`):
  ```
  cd backend
  dotnet build
  dotnet test          # 53 tests: unit tests per service + full HTTP integration tests via WebApplicationFactory
  dotnet run --project NftCerts    # real run: requires env vars below to be set
  ```
- Tests are offline: an in-memory SQLite connection, the local-disk stub storage, and a `ContractService` test double (subclassing the production class and overriding its `virtual` mint/read methods) stand in for the database file, Pinata, and a live chain. No live network or credentials required (NFR-4).
- Frontend: `cd frontend && npm install && npm run build`

## SixLabors.ImageSharp licensing note
`NftCerts.csproj` pins `SixLabors.ImageSharp` to `2.1.13`, the last Apache-2.0-licensed release. Versions 3.0+ require a Six Labors commercial license key (`SixLaborsLicenseKey`) checked at build time via their MSBuild target, which this offline/course environment doesn't have — do not bump past the 2.x line without first sorting out licensing.

## Storage provider switch
- `APP_STORAGE_PROVIDER` = `pinata` (default, real IPFS pinning) or `mock` (local-disk fake, no credentials needed). Active provider is logged at startup; the switch is applied in `Program.cs`'s DI registration for `IIpfsStorageService`.

## Required environment variables (backend, real end-to-end run only)
- `PINATA_JWT` (preferred) or `PINATA_API_KEY` + `PINATA_API_SECRET` — real Pinata pinning, only required when `APP_STORAGE_PROVIDER=pinata`. The app fails fast (`AppConfig.ValidateStartupConfig`) if missing in that mode; never commit these.
- `NFT_CONTRACT_ADDRESS`, `WEB3J_RPC_URL` (default `http://localhost:8545` for local Hardhat node), `MINTER_PRIVATE_KEY` (local Hardhat test account only — never a real-funds key).
- `SERVER_PORT` (default `8081`), `DB_PATH` (default `data/nft-certs.db`, SQLite file — created via `EnsureCreated()` at startup, not wiped between runs).
- All of the above are also loadable from a gitignored `.env` (or `../.env`) file; real environment variables always take precedence.

## Conventions
- Backend composition root: `NftCerts/Program.cs` wires DI (config, `NftCertsDbContext`, `IIpfsStorageService`/`IKycVerificationService` implementations, `ContractService`, `CertificateService`, PDF/DTO/upload-store singletons), registers controllers, and applies `ErrorHandlingMiddleware` before `MapControllers()`. `public partial class Program` at the end of the file is required for `WebApplicationFactory<Program>` in tests.
- Controllers use primary-constructor DI (`NftCerts/Api/Controllers.cs`); one controller class per resource, matching app01's controller-per-concern layout.
- Solidity: OpenZeppelin contracts only, no hand-rolled token-standard logic.
- Frontend: App Router, server components by default, `"use client"` only where wallet/browser APIs are needed.
- Known simplifications (full ADR in `docs/sdlc/10-glossary-and-decisions.md`): KYC is a mock auto-verifier behind `IKycVerificationService`; watermarking is metadata-based (EXIF), not steganographic; contracts run on a local Hardhat chain only. Storage is **real** Pinata pinning behind `IIpfsStorageService` (a local-disk stub implements the same interface for offline unit tests only).
