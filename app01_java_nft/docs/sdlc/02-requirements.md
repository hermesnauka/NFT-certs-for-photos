# 02 — Requirements

Priorities use the MoSCoW method: **M**ust have, **S**hould have, **C**ould have, **W**on't have
(this phase). Traceability points to the component that owns/satisfies the requirement; see
[03-architecture.md](./03-architecture.md) for component definitions.

## 1. Functional Requirements

| ID | Description | Priority | Traceability |
|----|--------------|----------|---------------|
| FR-1 | Accept a photograph upload (multipart file) and compute its SHA-256 hash server-side. | Must | `backend` — `HashingService`, `POST /api/uploads` |
| FR-2 | Embed the content hash and artist DID into the image's metadata (EXIF/XMP) as a watermark before storage. | Must | `backend` — `WatermarkService` |
| FR-3 | Pin the (watermarked) image bytes to IPFS via Pinata and obtain an immutable content URI. | Must | `backend` — `IpfsStorageService` (Pinata impl), `POST /api/artworks` |
| FR-4 | Build and pin an ERC-721/OpenSea-style metadata JSON document (title, description, image URI, attributes) to IPFS. | Must | `backend` — `IpfsStorageService`, `POST /api/artworks` |
| FR-5 | Let the artist declare curatorial metadata: title, description, medium, year of creation, royalty percentage. | Must | `frontend` — `/mint` page; `backend` — `Artwork` entity |
| FR-6 | Mint an ERC-721 token whose on-chain record stores the content hash and metadata URI. | Must | `contracts` — `PhotoCertificate.mintCertificate`; `backend` — `ContractService` |
| FR-7 | Reject minting if the same content hash has already been minted (duplicate-photo protection). | Must | `contracts` — `hashRegistered` mapping |
| FR-8 | Declare a per-token royalty recipient and percentage that is queryable via EIP-2981 `royaltyInfo`. | Must | `contracts` — `PhotoCertificate` (`ERC2981`) |
| FR-9 | Allow an authorized role to pause minting and transfers platform-wide. | Must | `contracts` — `Pausable`, `PAUSER_ROLE` |
| FR-10 | Allow a token owner to permanently burn their certificate. | Should | `contracts` — `ERC721Burnable` |
| FR-11 | Verify the minting requester's identity (DID + mock KYC) before allowing a mint request to proceed. | Must | `backend` — `KycVerificationService`, `POST /api/identity/verify` |
| FR-12 | Connect a browser wallet (MetaMask/injected; WalletConnect optional) to identify the artist's address. | Must | `frontend` — wagmi/viem wallet integration |
| FR-13 | Display a live, client-side SHA-256 hash of a dropped file before any upload occurs. | Must | `frontend` — `/mint` page, Web Crypto API |
| FR-14 | Show a dashboard of an artist's minted certificates, given their wallet address. | Must | `backend` — `GET /api/artists/{walletAddress}/dashboard`; `frontend` — `/` |
| FR-15 | Generate and serve a downloadable PDF certificate for a minted token, including blockchain and IPFS links. | Must | `backend` — `GET /api/certificates/{tokenId}/pdf` (OpenPDF) |
| FR-16 | Provide "List on OpenSea" / "View on Rarible" deep links on the certificate page. | Must | `frontend` — `/certificate/[id]` |
| FR-17 | Provide an educational panel explaining, in plain language, how the smart contract protects against piracy. | Should | `frontend` — `/certificate/[id]` educational panel |
| FR-18 | Provide a language switcher (EN/PL) with a flag icon, applied across the whole UI. | Must | `frontend` — flag switcher component; `backend` — `GET /api/i18n/{lang}` |
| FR-19 | Look up a certificate's full details by token ID. | Must | `backend` — `GET /api/certificates/{tokenId}` |
| FR-20 | Allow an authorized role to unpause the contract after a dispute is resolved. | Should | `contracts` — `unpause()`, `PAUSER_ROLE` |

## 2. Non-Functional Requirements

| ID | Description | Priority | Traceability |
|----|--------------|----------|---------------|
| NFR-1 | Hashing must be streaming/constant-memory so large image files do not exhaust backend heap. | Must | `backend` — `HashingService` |
| NFR-2 | Pinata credentials (`PINATA_JWT` or key/secret pair) must never be committed to source control and must be supplied via environment variables. | Must | `backend` config; [09-deployment-and-devops.md](./09-deployment-and-devops.md) |
| NFR-3 | The backend must fail fast with a clear error at startup (real profile) if required Pinata/Web3j env vars are missing, rather than failing silently on first request. | Must | `backend` — config validation |
| NFR-4 | Unit tests for backend and contracts must run with no live network dependency (no real Pinata calls, no live chain) using fakes/mocks. | Must | `backend` — local-disk fake `IpfsStorageService`; `contracts` — Hardhat in-memory network |
| NFR-5 | The minter private key used to sign mint transactions must be a well-known Hardhat test-only key for this phase, never a real-funds key. | Must | `backend` config; [09-deployment-and-devops.md](./09-deployment-and-devops.md) |
| NFR-6 | Smart contract state changes that affect ownership/royalties must emit events for off-chain indexing (`CertificateMinted`). | Must | `contracts` — `PhotoCertificate` |
| NFR-7 | All user-facing text must be resolvable in both English and Polish with no hard-coded strings in components. | Must | `frontend` i18n; `backend` — `GET /api/i18n/{lang}` |
| NFR-8 | The API must be documented with explicit request/response JSON contracts before backend implementation begins. | Must | [05-api-design.md](./05-api-design.md) |
| NFR-9 | Contract code must reuse audited OpenZeppelin implementations rather than hand-rolled token-standard logic. | Must | `contracts` — OpenZeppelin `ERC721`, `ERC2981`, `Pausable`, `ERC721Burnable`, `AccessControl` |
| NFR-10 | The system must be operable end-to-end on a developer laptop with no cloud infrastructure beyond Pinata's free tier. | Must | [09-deployment-and-devops.md](./09-deployment-and-devops.md) |
| NFR-11 | Backend and frontend must be independently buildable/testable (`mvn test`, `npm run build`) without one blocking the other. | Should | monorepo layout |
| NFR-12 | Threat surfaces (private key custody, API key leakage, replay minting, KYC spoofing, tampering, trust boundaries) must be documented with mitigations before implementation. | Must | [07-security-and-threat-model.md](./07-security-and-threat-model.md) |
| NFR-13 | Certificate PDF generation must not depend on a headless browser (keep the dependency footprint small and Java-native). | Should | `backend` — OpenPDF |
| NFR-14 | Contract gas usage for `mintCertificate` should be reviewed and kept reasonable for a single-artwork mint (no unbounded loops, no redundant storage writes). | Should | [06-smart-contract-design.md](./06-smart-contract-design.md) |
| NFR-15 | This documentation set itself must remain internally consistent and be updated if architecture decisions change in later phases. | Must | `docs/sdlc/` |

## 3. Out of Scope (Won't have, this phase)

| ID | Description | Reason |
|----|--------------|--------|
| W-1 | Public testnet or mainnet contract deployment | Course/reference-implementation scope; local Hardhat only |
| W-2 | Real KYC/identity-provider integration (Persona, Onfido, Civic, etc.) | Mocked behind `KycVerificationService`; swap-in point documented |
| W-3 | Steganographic (LSB) watermarking | Metadata-based watermarking chosen for reliability/maintainability |
| W-4 | Durable production database | H2 in-memory persistence for this phase |
| W-5 | Direct browser-to-contract minting (user wallet signs mint tx) | Backend-mediated minting via a single configured minter account |

## Related Documents

- [01-vision-and-scope.md](./01-vision-and-scope.md)
- [03-architecture.md](./03-architecture.md)
- [05-api-design.md](./05-api-design.md)
- [06-smart-contract-design.md](./06-smart-contract-design.md)
- [07-security-and-threat-model.md](./07-security-and-threat-model.md)
