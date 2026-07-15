# 10 — Glossary and Architecture Decision Records

## 1. Glossary

| Term | Definition |
|------|------------|
| **NFT (Non-Fungible Token)** | A blockchain token representing unique ownership of a specific asset (here, a photograph's certificate), as opposed to fungible/interchangeable tokens. |
| **ERC-721** | The Ethereum standard interface for non-fungible tokens; defines ownership, transfer, and approval semantics for unique tokens. |
| **ERC-1155** | A multi-token standard supporting both fungible and non-fungible tokens in one contract; considered but not chosen (see [06-smart-contract-design.md](./06-smart-contract-design.md) §1 — each photograph is a unique 1-of-1 certificate, fitting ERC-721 better). |
| **EIP-2981** | Ethereum Improvement Proposal defining a standard `royaltyInfo()` read function so marketplaces can query and honor creator royalties on secondary sales without a bespoke integration per platform. |
| **IPFS (InterPlanetary File System)** | A content-addressed, peer-to-peer distributed storage protocol; files are referenced by a hash-derived CID (Content Identifier) rather than a location, so the same content always resolves to the same address. |
| **Pinning** | The act of a node committing to keep a copy of IPFS content available/retrievable, since IPFS itself does not guarantee persistence without at least one pinning participant. |
| **Pinata** | A managed IPFS pinning service used by this platform to pin uploaded images and metadata JSON, avoiding the need to operate a self-hosted IPFS node. |
| **Arweave** | An alternative permanent-storage network using a one-time-payment, "endowment" model for storage; considered as an alternative to IPFS/Pinata but not chosen for this phase. |
| **Watermarking (metadata-based, this project)** | Embedding the content hash and artist DID directly into an image's EXIF/XMP metadata fields, making the authorship link human- and machine-readable without altering visible pixel data. |
| **Steganography** | Hiding data (e.g., a hash or identifier) within the pixel data itself (e.g., least-significant-bit encoding), such that it survives certain transformations but is harder to build, verify, and maintain reliably. Documented as a future enhancement, not implemented this phase. |
| **DID (Decentralized Identifier)** | A globally unique identifier (e.g., `did:key:...`) that does not depend on a centralized registry, used here to represent an artist's verifiable identity independent of any single KYC provider. |
| **KYC (Know Your Customer)** | The process of verifying the real-world identity of a party before allowing them to transact; here, verifying that a minting wallet belongs to the actual rights-holder of a photograph. |
| **AccessControl** | An OpenZeppelin contract pattern providing role-based permissions (`grantRole`/`revokeRole`/`onlyRole`) rather than a single-owner (`Ownable`) model. |
| **Pausable** | An OpenZeppelin contract pattern providing a circuit-breaker (`pause()`/`unpause()`) that can gate sensitive functions via a `whenNotPaused` modifier. |
| **Burnable (ERC721Burnable)** | An OpenZeppelin ERC-721 extension letting a token's owner permanently destroy (burn) it. |
| **Basis points (bps)** | A unit equal to 1/100th of a percent; used for royalty percentages (`royaltyFeeBasisPoints`) so fractional percentages (e.g., 7.5%) can be expressed as an integer (750). |
| **Web3j** | A Java/Kotlin library for interacting with Ethereum-compatible JSON-RPC nodes: building/signing transactions, calling contract functions, decoding events. |
| **FunctionEncoder** | A Web3j utility for manually ABI-encoding a smart-contract function call from its name and typed arguments, without relying on a generated Java contract wrapper class. |
| **ABI (Application Binary Interface)** | The standardized description of how to encode/decode calls and data for a smart contract, analogous to a function signature contract between caller and contract. |
| **Hardhat** | A Node.js-based Ethereum development environment providing a local test chain, compilation, deployment scripting, and a Chai/Mocha-based testing framework. |
| **CID (Content Identifier)** | The hash-derived address of a piece of content on IPFS; changes if the underlying bytes change, providing an independent tamper-evidence signal. |
| **MoSCoW** | A prioritization method: Must have, Should have, Could have, Won't have (this phase) — used in [02-requirements.md](./02-requirements.md). |
| **STRIDE** | A threat-modeling mnemonic: Spoofing, Tampering, Repudiation, Information Disclosure, Denial of Service, Elevation of Privilege — used in [07-security-and-threat-model.md](./07-security-and-threat-model.md). |

## 2. Architecture Decision Records (ADRs)

### ADR-1: Use Pinata for Decentralized Storage

- **Context**: The platform needs permanent, immutable storage for artwork images and metadata
  JSON (FR-3, FR-4). Options considered: self-hosted IPFS node, Arweave, or a managed pinning
  service (Pinata).
- **Decision**: Use Pinata's REST API (`pinFileToIPFS`, `pinJSONToIPFS`) as the storage backend,
  authenticated via `PINATA_JWT` (preferred) or an API key/secret pair.
- **Consequences**: Storage is real (not mocked) by default for the actual run path — this is
  intentionally not one of the phase's simplifications. Operating a self-hosted IPFS node or an
  Arweave bundler is avoided, reducing infrastructure burden for a course setting; Pinata's free
  tier is sufficient for development and grading. The trade-off is a dependency on a third-party
  managed service and its credentials (mitigated per
  [07-security-and-threat-model.md](./07-security-and-threat-model.md) T-2). A local-disk fake
  implementing the same `IpfsStorageService` interface is available for two purposes: (a) wired
  automatically under the `test` Spring profile for offline unit tests (NFR-4), and (b) selectable
  at runtime via the `app.storage.provider` config property (`pinata` default, `mock` alternative —
  see [09-deployment-and-devops.md](./09-deployment-and-devops.md) §2) for demos or environments
  without Pinata credentials. When `app.storage.provider=mock`, the Pinata credential fail-fast
  check is skipped entirely; the application must log which provider is active at startup so the
  active mode is never ambiguous.

### ADR-2: Metadata-Based Watermarking Instead of Steganography

- **Context**: The spec calls for "digital watermarking or steganography combined with metadata"
  to link a file indisputably to the artist's identity (spec §1). Steganographic (e.g., LSB)
  encoding survives certain image transformations but is harder to implement reliably, harder to
  verify, and more fragile against common operations (recompression, resizing) than metadata
  fields.
- **Decision**: Implement watermarking by embedding the content hash and artist DID into the
  image's EXIF/XMP metadata via Apache Commons Imaging, rather than pixel-level steganography.
- **Consequences**: Simpler, more maintainable, and fully testable (round-trip read/write
  assertions, see [08-test-plan.md](./08-test-plan.md)). The trade-off is that metadata can be
  stripped by some image-processing pipelines or marketplace re-encoding, whereas steganographic
  data is embedded in pixel values. Since the SHA-256 hash and IPFS CID already provide independent
  tamper-evidence (see [07-security-and-threat-model.md](./07-security-and-threat-model.md) T-5),
  metadata-based watermarking is judged sufficient for this phase. Steganography is documented as a
  future enhancement, not scheduled.

### ADR-3: Mock KYC Verification Service

- **Context**: The spec requires verifying that a minting entity is the rightful copyright holder
  (DID and KYC, spec §4), but integrating a real identity provider (Persona, Onfido, Civic) is a
  significant external dependency and cost/compliance surface not appropriate for a course
  reference implementation.
- **Decision**: Implement `KycVerificationService` as an interface with a mock, auto-approving
  implementation for this phase.
- **Consequences**: The mint flow can be fully exercised end-to-end without a real identity
  provider account or cost. This is an explicitly accepted risk (T-4 in
  [07-security-and-threat-model.md](./07-security-and-threat-model.md)) — a fabricated
  wallet/DID/email is currently accepted as "verified." The interface boundary means a real
  provider can be substituted later without changing `POST /api/identity/verify`'s contract or any
  caller code.

### ADR-4: Local Hardhat Chain Only (No Testnet/Mainnet Deploy)

- **Context**: The platform needs an EVM-compatible chain to deploy `PhotoCertificate.sol` and mint
  tokens against. Public testnets require faucet funds and network reliability; mainnet requires
  real funds, an audit, and production key custody — none appropriate for this phase of a training
  course.
- **Decision**: Deploy and test only against a local Hardhat network for this phase; document (but
  do not execute) a promotion path to testnet and mainnet.
- **Consequences**: Zero cost, fully reproducible, and safe for a training environment — no risk of
  real-fund loss via `MINTER_PRIVATE_KEY` (which is a well-known Hardhat test account key, see
  [07-security-and-threat-model.md](./07-security-and-threat-model.md) T-1). The trade-off is that
  "Etherscan" links and marketplace deep links are illustrative/placeholder rather than resolving
  to real public listings (see [05-api-design.md](./05-api-design.md), note under §3). The
  promotion path is documented in
  [09-deployment-and-devops.md](./09-deployment-and-devops.md) §4 for future phases.

### ADR-5: H2 In-Memory Persistence

- **Context**: The backend needs to persist `Artwork`, `Certificate`, and `ArtistIdentity` records.
  A durable production database (PostgreSQL, MySQL) would require additional infrastructure setup
  not justified for this phase's local-development/course scope.
- **Decision**: Use Spring Data JPA with H2 running in in-memory mode.
- **Consequences**: Zero external dependency for local development and grading; trivially reset
  between test runs. The explicit, accepted limitation is that all data is lost on backend restart
  — documented in [09-deployment-and-devops.md](./09-deployment-and-devops.md) §5 and in
  `AGENTS.md`. On-chain data (via `tokenContentHash`, `royaltyInfo`, ownership) remains the source
  of truth and is independently recoverable from the chain even if the H2-backed cache is lost,
  though full reconciliation tooling (e.g., replaying `CertificateMinted` events) is not built in
  this phase.

### ADR-6: Manual Web3j ABI Encoding via `FunctionEncoder` Instead of Generated Wrappers

- **Context**: Web3j supports generating typed Java wrapper classes from a contract's ABI/bytecode
  at build time (`web3j generate solidity`), which gives compile-time-checked contract calls but
  couples the backend's build to the contracts module's compiled artifacts and requires a codegen
  step to stay in sync with contract changes.
- **Decision**: Have `ContractService` manually construct the `mintCertificate` call using Web3j's
  `FunctionEncoder` with explicit `Function`/`Type` definitions, rather than using generated
  contract wrapper classes.
- **Consequences**: Decouples the backend module's build from the contracts module's compiled
  output — the backend can be built and unit-tested independently (NFR-11) without a contracts
  codegen dependency in its build path. The explicit ABI encoding is also more transparent for
  teaching purposes: the encoding logic is visible in Java code rather than hidden in generated
  classes. The trade-off is losing compile-time type safety on the call arguments and needing to
  manually keep the encoding in sync if the contract's function signature changes; this is
  mitigated by the `ContractService` unit test that asserts the encoded payload against a known
  expected value (see [08-test-plan.md](./08-test-plan.md)).

## Related Documents

- [01-vision-and-scope.md](./01-vision-and-scope.md)
- [02-requirements.md](./02-requirements.md)
- [03-architecture.md](./03-architecture.md)
- [06-smart-contract-design.md](./06-smart-contract-design.md)
- [07-security-and-threat-model.md](./07-security-and-threat-model.md)
- [09-deployment-and-devops.md](./09-deployment-and-devops.md)
