# 07 — Security and Threat Model

This document applies a STRIDE-style analysis (Spoofing, Tampering, Repudiation, Information
Disclosure, Denial of Service, Elevation of Privilege) to the trust boundaries introduced in
[03-architecture.md](./03-architecture.md). Each threat lists the affected boundary, the STRIDE
category, and the mitigation adopted for this phase.

## 1. Threat Table

| # | Threat | Boundary | STRIDE | Mitigation |
|---|--------|----------|--------|------------|
| T-1 | Minter private key (`MINTER_PRIVATE_KEY`) is leaked or exfiltrated, allowing an attacker to mint arbitrary certificates. | Backend ↔ Local Hardhat Node | Spoofing, Elevation of Privilege | Key is a well-known **Hardhat test-only** key for this phase — it controls no real funds and the contract is never deployed beyond a local network (NFR-5). Documented as a hard blocker before any real deployment: production must use a hardware wallet, KMS, or HSM-backed signer, never a plaintext env var. |
| T-2 | Pinata API credentials (`PINATA_JWT` / API key+secret) are committed to source control or logged, allowing an attacker to pin/unpin content or exhaust the account's quota. | Backend ↔ Pinata | Information Disclosure, Denial of Service | Credentials are supplied only via environment variables / gitignored `.env`, never hard-coded (NFR-2). Backend fails fast at startup if missing (NFR-3), avoiding silent fallback to an insecure state. `.gitignore` explicitly excludes `.env` files (see `app01_java_nft/.gitignore`). |
| T-3 | Duplicate/replay minting: an attacker re-submits the same image (or a byte-identical copy) to mint a second certificate under a different wallet address, diluting the original artist's provenance claim. | Backend REST API + on-chain contract | Spoofing, Tampering | Two-layer defense: (a) backend checks `sha256Hash` uniqueness before pinning/minting and returns `409` (see [05-api-design.md](./05-api-design.md)); (b) the contract's `hashRegistered` mapping independently rejects a duplicate `contentHash` at the `mintCertificate` call, so the guarantee holds even if the backend check is bypassed (see [06-smart-contract-design.md](./06-smart-contract-design.md)). |
| T-4 | KYC spoofing: an attacker supplies a fabricated wallet address / DID / email to `POST /api/identity/verify` and obtains a "verified" status without proving identity, since the mock service auto-approves. | Backend `KycVerificationService` | Spoofing | Explicitly accepted risk for this phase — documented as a mock (see ADR in [10-glossary-and-decisions.md](./10-glossary-and-decisions.md)). The service is isolated behind an interface so swapping in a real KYC/DID provider (Persona/Onfido/Civic) requires no changes to callers. This must be resolved before any real-value deployment. |
| T-5 | Image tampering after hashing: a file is altered (recompressed, cropped, metadata-stripped) after its SHA-256 hash is computed but before/after pinning, breaking the hash-to-content link. | Backend upload pipeline | Tampering | Hash is computed from the exact byte stream received in `POST /api/uploads` (NFR-1, streaming hash), and watermarking (EXIF/XMP embedding of the hash + DID) is applied to that same file before it is pinned — the pinned bytes on IPFS are the watermarked, hashed file, not a re-encoded copy. Any downstream re-encoding (e.g., a marketplace generating a thumbnail) does not affect the canonical pinned original, since IPFS content addressing (CID) itself changes if bytes change, giving an independent tamper-evidence signal beyond the embedded hash. |
| T-6 | Frontend–backend trust boundary: the frontend performs a client-side SHA-256 hash and metadata validation for UX purposes, but a malicious client could submit a mismatched hash or invalid metadata directly to the API. | Browser ↔ Backend | Tampering, Spoofing | All validation is re-executed server-side; the client-side hash (Web Crypto API) is purely informational/educational (FR-13) and is never trusted as the certificate's hash of record — the backend always recomputes SHA-256 from the uploaded bytes it receives. Metadata fields (royalty bps range, year, address format) are validated server-side regardless of client-side form validation. |
| T-7 | Denial of service via large or repeated file uploads exhausting backend memory or Pinata quota. | Browser ↔ Backend, Backend ↔ Pinata | Denial of Service | Streaming hash computation avoids loading the full file into a `byte[]` unnecessarily (NFR-1); upload size limits enforced at the Spring Boot multipart config layer (`413` response, see [05-api-design.md](./05-api-design.md)). Full rate-limiting is out of scope this phase and flagged as a future hardening item. |
| T-8 | Role misconfiguration: `MINTER_ROLE` or `PAUSER_ROLE` granted to an unintended address at deploy time, allowing unauthorized minting or pausing. | Contract deployment | Elevation of Privilege | Role grants happen explicitly in the deploy script, reviewed as part of Hardhat test coverage (access-control test cases, see [08-test-plan.md](./08-test-plan.md)); `DEFAULT_ADMIN_ROLE` remains solely with the deployer account so any incorrect grant can be revoked. |
| T-9 | Repudiation: an artist denies having requested a mint, disputing ownership of a certificate after the fact. | Backend ↔ Contract | Repudiation | Every mint is tied to an on-chain transaction (`txHash`) signed by the platform's minter key on behalf of a KYC-"verified" wallet address, and the backend persists the request's `recipientAddress` and the resulting `Certificate` record; the on-chain `CertificateMinted` event provides an immutable, timestamped record independent of backend database state. |
| T-10 | Information disclosure via verbose error messages (e.g., stack traces, internal file paths) returned from the API. | Backend ↔ Browser | Information Disclosure | Standardized error response shape (see [05-api-design.md](./05-api-design.md) §9) with a generic `message` field; internal exception details are logged server-side only, not returned to clients. |

## 2. Summary of Accepted Risks (this phase)

The following risks are explicitly accepted as documented simplifications for a course/reference
implementation and must be revisited before any production or real-value deployment (cross-refs to
[10-glossary-and-decisions.md](./10-glossary-and-decisions.md) ADRs):

- Mock KYC (T-4) — no real identity proofing.
- Local-only minter key (T-1) — not production key-management practice.
- No rate limiting (T-7) — acceptable for local/course traffic volumes only.

## 3. Non-Threats (explicitly out of scope)

- Smart-contract upgrade-proxy security (the contract is not upgradeable in this phase — a fixed,
  audited-pattern contract is simpler to reason about for a course setting).
- Marketplace-side security (OpenSea/Rarible's own infrastructure) — outside this system's
  boundary; the platform only produces deep links (FR-16).

## Related Documents

- [03-architecture.md](./03-architecture.md) — trust boundaries referenced above
- [05-api-design.md](./05-api-design.md) — error shapes, endpoint validation
- [06-smart-contract-design.md](./06-smart-contract-design.md) — duplicate-hash and role mechanics
- [09-deployment-and-devops.md](./09-deployment-and-devops.md) — secret handling in practice
- [10-glossary-and-decisions.md](./10-glossary-and-decisions.md) — ADRs for accepted-risk simplifications
