# 08 — Test Plan

Test strategy is layered: smart-contract tests (Hardhat/Chai), backend unit tests (JUnit 5 +
Mockito + MockMvc), frontend build/lint checks, and a manual end-to-end smoke test that exercises
real Pinata and a real local chain together. Unit-level automated tests in contracts/backend must
never require live network access or real credentials (NFR-4).

## 1. Smart Contract Tests (Hardhat + Chai + ethers), `contracts/test/`

Run via `npx hardhat test` against Hardhat's in-memory network (no real chain, no real gas cost).

| Case | Description |
|------|--------------|
| Mint success | `mintCertificate` called by an address with `MINTER_ROLE` succeeds, returns an incrementing `tokenId`, sets owner correctly, emits `CertificateMinted` with correct args. |
| Duplicate-hash rejection | A second `mintCertificate` call with a `contentHash` already registered reverts with the expected reason string; first mint's data is untouched. |
| Royalty math | `royaltyInfo(tokenId, salePrice)` returns `(royaltyRecipient, salePrice * royaltyFeeBasisPoints / 10000)` for various `salePrice`/`royaltyFeeBasisPoints` combinations, including edge cases (0 bps, max 10000 bps). |
| Token content hash read | `tokenContentHash(tokenId)` returns the exact `bytes32` passed at mint time. |
| Pause blocks mint | Calling `mintCertificate` while paused reverts (`whenNotPaused`); unrelated view functions still work while paused. |
| Pause blocks transfer | Calling `transferFrom`/`safeTransferFrom` on an existing token while paused reverts; transfers succeed again after `unpause()`. |
| Burn | Token owner calls `burn(tokenId)`; token no longer exists (`ownerOf` reverts); `hashRegistered[contentHash]` remains `true` after burn (re-mint of the same hash still reverts). |
| Access control — minter | An address without `MINTER_ROLE` calling `mintCertificate` reverts with an `AccessControl` unauthorized error. |
| Access control — pauser | An address without `PAUSER_ROLE` calling `pause()`/`unpause()` reverts. |
| Access control — admin | Only an address with `DEFAULT_ADMIN_ROLE` can grant/revoke `MINTER_ROLE`/`PAUSER_ROLE`; granting/revoking is verified via `hasRole`. |
| `supportsInterface` | Returns `true` for `IERC721`, `IERC2981`, and `IAccessControl` interface IDs. |
| Metadata URI | `tokenURI(tokenId)` returns the exact `metadataURI` string passed at mint time. |

## 2. Backend Tests (JUnit 5 + Mockito + MockMvc), `backend/src/test/java/...`

Run via `mvn test`. All external dependencies (Pinata, chain RPC) are faked or mocked — no live
network required (NFR-4). The local-disk fake `IpfsStorageService` implementation exists solely to
support these tests and is never wired into the real-run Spring profile (see
[03-architecture.md](./03-architecture.md), [10-glossary-and-decisions.md](./10-glossary-and-decisions.md)).

| Layer | Test focus |
|-------|-------------|
| `HashingService` (unit) | Streaming SHA-256 hash of a known test file matches a precomputed expected hash; large-file input does not require loading the whole file into memory (tested via a bounded-buffer assertion or a large synthetic stream). |
| `WatermarkService` (unit) | Embedding content hash + DID into EXIF/XMP is round-trippable — reading the watermark back from the output file yields the same hash/DID that were written in. |
| `IpfsStorageService` fake (unit) | The local-disk fake implements the same interface contract as the Pinata implementation (same method signatures, same success/failure shape) so it is a valid substitute in tests. |
| `KycVerificationService` mock (unit) | Auto-approve behavior is deterministic and covers malformed-input validation (`400` cases) independent of any real provider. |
| `ContractService` (unit, Web3j mocked) | `FunctionEncoder`-built call data for `mintCertificate` matches the expected ABI encoding for a fixed set of inputs (regression-style assertion against a known encoded payload); duplicate-hash on-chain revert is mapped to the correct API-level `409`. |
| `CertificatePdfService` (unit) | Generated PDF byte stream is non-empty, valid PDF (parseable), and contains expected text fragments (title, hash, links) via a text-extraction check. |
| `POST /api/uploads` (MockMvc) | Multipart upload returns `fileId`/`sha256Hash`/`sizeBytes`; unsupported type returns `400`; oversized file returns `413`. |
| `POST /api/artworks` (MockMvc, fake storage) | Valid metadata + known `fileId` returns `201` with IPFS URIs from the fake; unknown `fileId` returns `404`; out-of-range royalty bps returns `422`. |
| `POST /api/artworks/{id}/mint` (MockMvc, mocked `ContractService`) | Successful mint path returns `200` with full `CertificateDto`; unverified artist returns `403`; duplicate content hash returns `409`; artwork not yet pinned returns `422`. |
| `GET /api/certificates/{tokenId}` (MockMvc) | Known token returns `CertificateDto`; unknown token returns `404`. |
| `GET /api/certificates/{tokenId}/pdf` (MockMvc) | Response has `Content-Type: application/pdf` and a non-empty body. |
| `GET /api/artists/{walletAddress}/dashboard` (MockMvc) | Wallet with certificates returns populated list; unknown wallet returns empty list with `totalCertificates: 0`, not an error. |
| `GET /api/i18n/{lang}` (MockMvc) | `en`/`pl` return populated maps with matching key sets; unsupported `lang` returns `404`. |
| `POST /api/identity/verify` (MockMvc) | Valid input returns `verified: true`; malformed wallet address/DID returns `400`. |
| Config validation (unit) | Application context fails to start (or a startup validator throws) when `PINATA_JWT`/key-secret and Web3j env vars are absent under the "real" profile — verifies NFR-3's fail-fast behavior without actually contacting Pinata/chain. |

## 3. Frontend Checks

| Check | Command | Purpose |
|-------|---------|---------|
| Lint | `npm run lint` | TypeScript/ESLint rule compliance across `app/`, `components/`, `lib/`. |
| Build | `npm run build` | Next.js production build succeeds (type-checking, static/server component boundaries valid, no broken imports). |
| (Future phase) Component tests | — | Not yet defined; deferred to the implementation phase once components exist. |

## 4. Manual End-to-End Smoke Test

Performed manually once contracts/backend/frontend are implemented, using **real** Pinata
credentials and a **real** local Hardhat chain (this is the one path in the whole test strategy
that is intentionally not fully automated/faked, because it validates the real integrations).

1. Start a local Hardhat node: `cd contracts && npx hardhat node`.
2. Deploy `PhotoCertificate.sol` to the running local node via the Hardhat deploy script; note the
   deployed contract address and the minter account's address (must match `MINTER_PRIVATE_KEY`
   used by the backend).
3. Start the backend with the "real" Spring profile active and the following environment variables
   set: `PINATA_JWT` (real Pinata credential), `WEB3J_RPC_URL=http://localhost:8545`,
   `MINTER_PRIVATE_KEY` (the local Hardhat test account key), `NFT_CONTRACT_ADDRESS` (from step 2).
   Confirm the backend fails to start with a clear error if any of these are missing (validates
   NFR-3), then confirm it starts cleanly once they are all set.
4. Start the frontend (`npm run dev`), pointed at the backend's base URL.
5. Walk the full user journey in the browser:
   a. Connect a browser wallet (MetaMask) on the `/mint` page.
   b. Drag and drop a sample photograph; confirm the live client-side SHA-256 hash is displayed
      (FR-13).
   c. Fill in title, description, medium, year, royalty percentage; submit.
   d. Confirm the artwork is pinned to Pinata (check the returned `imageIpfsUri`/`metadataIpfsUri`
      resolve via a public IPFS gateway).
   e. Trigger minting; confirm a `CertificateDto` is returned with a valid `txHash` against the
      local chain.
   f. Navigate to `/certificate/[id]`; confirm the PDF download works, the IPFS links resolve, and
      the OpenSea/Rarible deep links are correctly formed (they need not resolve to a real listing
      since the contract is not deployed publicly).
   g. Switch the UI language via the flag switcher; confirm all visible strings update to Polish
      and back to English.
   h. Re-upload the exact same photograph and attempt to mint again; confirm the duplicate-hash
      rejection is surfaced clearly in the UI (backed by the API's `409`).

## 5. Explicitly Out of Scope This Phase

- **Testnet deployment testing.** No Sepolia/Goerli/other public testnet deployment or
  verification is performed; all tests target the local Hardhat network only (see
  [09-deployment-and-devops.md](./09-deployment-and-devops.md)).
- **Real KYC provider testing.** `KycVerificationService` is exercised only in its mock,
  auto-approving form; no integration tests against a real identity provider exist.
- **Steganography testing.** Since watermarking in this phase is metadata-based (EXIF/XMP), no
  steganographic (LSB) embedding/extraction tests are written; this is deferred along with the
  feature itself (see [10-glossary-and-decisions.md](./10-glossary-and-decisions.md)).
- **Load/performance testing** and **security penetration testing** beyond the threat model in
  [07-security-and-threat-model.md](./07-security-and-threat-model.md).

## Related Documents

- [05-api-design.md](./05-api-design.md)
- [06-smart-contract-design.md](./06-smart-contract-design.md)
- [07-security-and-threat-model.md](./07-security-and-threat-model.md)
- [09-deployment-and-devops.md](./09-deployment-and-devops.md)
