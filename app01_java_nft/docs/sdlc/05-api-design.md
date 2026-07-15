# 05 — API Design

Base path: **`/api`**. All request/response bodies are JSON unless noted (file upload is
multipart, PDF download is binary). This document is the fixed contract for later backend
implementation — endpoint shapes here must not be reinvented in code without updating this file
first.

Related: [06-smart-contract-design.md](./06-smart-contract-design.md) (what `mint` calls into),
[04-data-model.md](./04-data-model.md) (entities behind these DTOs).

## 1. `POST /api/uploads`

Uploads a photograph, computes its SHA-256 hash, and applies metadata watermarking. Does **not**
pin to Pinata yet — that happens in step 2 (`/api/artworks`), once the artist confirms metadata.

**Request**: `multipart/form-data`, field `file` (the image).

**Response `200 OK`:**
```json
{
  "fileId": "5b1f9c2a-2e3f-4a3e-9c2a-1234567890ab",
  "originalFilename": "lighthouse-at-dusk.jpg",
  "sha256Hash": "9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08",
  "sizeBytes": 8421376
}
```

**Errors**: `400` unsupported file type / empty file; `413` file too large.

## 2. `POST /api/artworks`

Confirms artist-provided metadata for a previously uploaded file, then pins the (watermarked)
image and a generated metadata JSON document to IPFS via Pinata.

**Request:**
```json
{
  "fileId": "5b1f9c2a-2e3f-4a3e-9c2a-1234567890ab",
  "title": "Lighthouse at Dusk",
  "description": "Long-exposure coastal photograph, silver gelatin print digitized at 100MP.",
  "medium": "Archival pigment print",
  "yearCreated": 2024,
  "royaltyPercentageBps": 750,
  "artistWalletAddress": "0xA1b2C3d4E5f60718293a4b5c6d7e8f901234567",
  "artistDid": "did:key:z6MkhaXgBZDvotDkL5257faiztiGiC2QtKLGpbnnEGta2doK"
}
```

**Response `201 Created`:**
```json
{
  "artworkId": "7c2e4a10-88b1-4f3a-9d0e-abcdef012345",
  "sha256Hash": "9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08",
  "imageIpfsUri": "ipfs://bafybeigdyrztqcz3vjm... (image CID)",
  "metadataIpfsUri": "ipfs://bafybeihdk3vqz9c2x8v... (metadata CID)",
  "status": "PINNED"
}
```

**Errors**: `404` unknown `fileId`; `422` `royaltyPercentageBps` out of range (0–10000); `502`
Pinata unavailable/misconfigured (real profile only — see NFR-3 in
[02-requirements.md](./02-requirements.md)).

## 3. `POST /api/artworks/{artworkId}/mint`

Triggers on-chain minting via `ContractService`, which calls `PhotoCertificate.mintCertificate`
(see [06-smart-contract-design.md](./06-smart-contract-design.md)). Requires the artist's wallet
address to have a `verified` `ArtistIdentity` record (see `POST /api/identity/verify`).

**Request:**
```json
{
  "recipientAddress": "0xA1b2C3d4E5f60718293a4b5c6d7e8f901234567"
}
```

**Response `200 OK`** (`CertificateDto`):
```json
{
  "tokenId": 42,
  "artworkId": "7c2e4a10-88b1-4f3a-9d0e-abcdef012345",
  "title": "Lighthouse at Dusk",
  "contentHashHex": "9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08",
  "contractAddress": "0x5FbDB2315678afecb367f032d93F642f64180aa",
  "txHash": "0x8b3e1f6c...d4a9",
  "ownerAddress": "0xA1b2C3d4E5f60718293a4b5c6d7e8f901234567",
  "royaltyPercentageBps": 750,
  "imageIpfsUri": "ipfs://bafybeigdyrztqcz3vjm...",
  "metadataIpfsUri": "ipfs://bafybeihdk3vqz9c2x8v...",
  "etherscanUrl": "http://localhost:8545/tx/0x8b3e1f6c...d4a9",
  "openSeaUrl": "https://testnets.opensea.io/assets/0x5FbDB2315678afecb367f032d93F642f64180aa/42",
  "raribleUrl": "https://rarible.com/token/0x5FbDB2315678afecb367f032d93F642f64180aa:42"
}
```

**Errors**: `404` unknown `artworkId`; `403` artist not KYC-verified; `409` `contentHash` already
minted (duplicate protection — mirrors the on-chain revert); `422` artwork not yet `PINNED`;
`502` chain unreachable / minter account misconfigured.

> Note: `etherscanUrl`/`openSeaUrl`/`raribleUrl` are built from configured base URLs per
> environment; against local Hardhat, the "Etherscan" link points at a local block explorer or is
> a placeholder, since there is no public explorer for a local chain (see
> [09-deployment-and-devops.md](./09-deployment-and-devops.md)).

## 4. `GET /api/certificates/{tokenId}`

Returns the same `CertificateDto` shape as the mint response, read from the backend's persisted
`Certificate` record (see [04-data-model.md](./04-data-model.md)).

**Response `200 OK`**: `CertificateDto` (see above).

**Errors**: `404` unknown `tokenId`.

## 5. `GET /api/certificates/{tokenId}/pdf`

Returns the downloadable certificate document.

**Response `200 OK`**: `Content-Type: application/pdf`, binary body. PDF contains: title,
artist, content hash, IPFS links, transaction link, royalty terms, and a short authenticity
statement (see [06-smart-contract-design.md](./06-smart-contract-design.md) for what guarantees
this PDF is communicating).

**Errors**: `404` unknown `tokenId`.

## 6. `GET /api/artists/{walletAddress}/dashboard`

**Response `200 OK`:**
```json
{
  "walletAddress": "0xA1b2C3d4E5f60718293a4b5c6d7e8f901234567",
  "certificates": [
    {
      "tokenId": 42,
      "artworkId": "7c2e4a10-88b1-4f3a-9d0e-abcdef012345",
      "title": "Lighthouse at Dusk",
      "contentHashHex": "9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08",
      "contractAddress": "0x5FbDB2315678afecb367f032d93F642f64180aa",
      "txHash": "0x8b3e1f6c...d4a9",
      "ownerAddress": "0xA1b2C3d4E5f60718293a4b5c6d7e8f901234567",
      "royaltyPercentageBps": 750,
      "imageIpfsUri": "ipfs://bafybeigdyrztqcz3vjm...",
      "metadataIpfsUri": "ipfs://bafybeihdk3vqz9c2x8v...",
      "etherscanUrl": "http://localhost:8545/tx/0x8b3e1f6c...d4a9",
      "openSeaUrl": "https://testnets.opensea.io/assets/0x5FbDB2315678afecb367f032d93F642f64180aa/42",
      "raribleUrl": "https://rarible.com/token/0x5FbDB2315678afecb367f032d93F642f64180aa:42"
    }
  ],
  "totalCertificates": 1
}
```

**Errors**: none for an unknown/empty wallet — returns `certificates: []`, `totalCertificates: 0`.

## 7. `GET /api/i18n/{lang}`

`lang` path variable is `en` or `pl`.

**Response `200 OK`** (flat JSON message map, illustrative subset):
```json
{
  "dashboard.title": "My Certificates",
  "mint.dropzone.label": "Drag and drop your photograph here",
  "mint.hash.explainer": "This is your artwork's digital fingerprint.",
  "certificate.viewOnOpenSea": "List on OpenSea",
  "certificate.viewOnRarible": "View on Rarible"
}
```

**Errors**: `404` unsupported `lang` (anything other than `en`/`pl`).

## 8. `POST /api/identity/verify`

Mock KYC/DID verification. Always succeeds in this phase (`KycVerificationService` auto-approves);
see [10-glossary-and-decisions.md](./10-glossary-and-decisions.md) ADR for why, and the swap-in
point for a real provider.

**Request:**
```json
{
  "walletAddress": "0xA1b2C3d4E5f60718293a4b5c6d7e8f901234567",
  "did": "did:key:z6MkhaXgBZDvotDkL5257faiztiGiC2QtKLGpbnnEGta2doK",
  "email": "artist@example.com"
}
```

**Response `200 OK`:**
```json
{
  "verified": true,
  "did": "did:key:z6MkhaXgBZDvotDkL5257faiztiGiC2QtKLGpbnnEGta2doK",
  "walletAddress": "0xA1b2C3d4E5f60718293a4b5c6d7e8f901234567"
}
```

**Errors**: `400` malformed wallet address or DID.

## 9. Error Response Shape (all endpoints)

```json
{
  "timestamp": "2026-07-15T10:15:30Z",
  "status": 404,
  "error": "Not Found",
  "message": "No artwork found with id 7c2e4a10-88b1-4f3a-9d0e-abcdef012345",
  "path": "/api/artworks/7c2e4a10-88b1-4f3a-9d0e-abcdef012345/mint"
}
```

## Related Documents

- [04-data-model.md](./04-data-model.md)
- [06-smart-contract-design.md](./06-smart-contract-design.md)
- [07-security-and-threat-model.md](./07-security-and-threat-model.md)
- [08-test-plan.md](./08-test-plan.md)
