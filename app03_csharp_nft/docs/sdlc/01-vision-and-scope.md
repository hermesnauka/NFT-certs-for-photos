# 01 — Vision and Scope

## 1. Problem Statement

Independent fine-art photographers who sell limited-edition digital or print-backed works face three
compounding problems when they try to establish provenance and monetize their work on public NFT
marketplaces (OpenSea, Rarible, Blur):

1. **No cryptographic proof of authorship or originality.** A photograph can be copied,
   re-uploaded, and re-minted by anyone. Without a verifiable "digital fingerprint," a buyer has no
   way to distinguish the artist's genuine certificate from a duplicate or a screenshot.
2. **No durable, tamper-evident storage.** Files hosted on a photographer's own website or a
   generic cloud bucket can disappear, be altered, or go offline, silently breaking the link between
   the sold NFT and the artwork it represents.
3. **No enforceable royalty stream on resale.** Marketplaces vary in how (or whether) they honor
   creator royalties on secondary sales, and a plain ERC-721 contract carries no on-chain, standard
   way to declare a royalty split that marketplaces can query.

Photographers are not blockchain engineers. They need a guided, web-based workflow that turns a
photo file into a marketplace-ready, royalty-bearing, provably-authentic NFT certificate, without
requiring them to understand Solidity, IPFS pinning, or wallet key management beyond connecting a
browser wallet.

## 2. Vision

Build **NFT Certs for Photos**, a creator portal and supporting backend/Web3 stack that lets an
independent fine-art photographer:

- Upload a high-resolution photograph.
- See a live SHA-256 "digital fingerprint" of the file as it is protected.
- Attach curatorial metadata (title, description, medium, year, royalty percentage).
- Mint an ERC-721 NFT certificate that embeds an on-chain content hash, an EIP-2981 royalty
  declaration, and a link to the artwork and its metadata pinned permanently on IPFS.
- Download an elegant, shareable PDF certificate of authenticity linking to the blockchain
  transaction and the IPFS assets.
- Move directly from a successful mint to listing the certificate on OpenSea or viewing it on
  Rarible.
- Use the entire application in either English or Polish via a simple flag-icon switcher.

This repository (`app01_java_nft`) is the **reference implementation** for the course: a Java/Spring
Boot backend, a Solidity/Hardhat contract layer, and a Next.js frontend, built incrementally and
documented before any code is written (this SDLC set is written first, deliberately).

## 3. Goals

| # | Goal |
|---|------|
| G1 | Give every minted certificate an immutable, verifiable link between the image bytes (SHA-256 hash) and the artist's declared identity. |
| G2 | Guarantee the artwork and its metadata survive independently of this application, via permanent decentralized storage (IPFS through Pinata). |
| G3 | Make royalties a first-class, standards-compliant on-chain fact (EIP-2981) rather than a marketplace-specific side agreement. |
| G4 | Give the platform operator a legal/compliance safety valve (pause, burn) without undermining the decentralized, non-custodial nature of ownership. |
| G5 | Make the artist-verification step (KYC/DID) a pluggable boundary so a real identity provider can be swapped in later without redesigning the flow. |
| G6 | Deliver a bilingual (EN/PL) UI suitable for a Polish-based training audience and their international peers. |
| G7 | Produce a documentation-first SDLC artifact set usable as a teaching reference for AI-assisted software delivery. |

## 4. Non-Goals (this phase)

- **No mainnet or public testnet deployment.** All smart-contract work targets a local Hardhat
  network only. See [09-deployment-and-devops.md](./09-deployment-and-devops.md).
- **No real KYC/identity verification provider integration.** A mock service auto-approves
  requests; the interface is designed for a future real provider. See
  [10-glossary-and-decisions.md](./10-glossary-and-decisions.md).
- **No steganographic watermarking.** Metadata-based watermarking (EXIF/XMP) is used instead;
  steganography is documented as a future enhancement.
- **No durable production database.** Backend persistence uses H2 in-memory storage for this
  phase; data does not survive a restart.
- **No direct browser-to-contract minting.** All minting is backend-mediated through a single
  configured minter account, not a user's connected wallet signing the mint transaction directly.
- **No secondary-market smart contract (marketplace, auction, escrow).** The scope is
  certificate minting and viewing; actual sale execution happens on third-party marketplaces.
- **No mobile app.** The frontend is a responsive web application only.

## 5. Target Users

- **Primary: Independent fine-art photographers** selling limited-edition works who want
  marketplace-ready NFT certificates without hiring blockchain engineers. They are comfortable with
  a browser wallet (MetaMask) but not with smart-contract tooling.
- **Secondary: Collectors/buyers** who view a certificate page to verify authenticity, royalty
  terms, and provenance before purchasing on a public marketplace. They are read-only consumers of
  this system (no login, no minting).
- **Tertiary: Platform operator/administrator** (course instructor role, in this reference
  implementation) who holds `PAUSER_ROLE`/`DEFAULT_ADMIN_ROLE` and can pause the contract in
  response to a legal copyright claim.

## 6. Success Criteria

This phase (documentation) is complete when the 10 documents in `docs/sdlc/` describe a coherent,
buildable system with no open contradictions between requirements, API design, data model, and
smart-contract design. The overall product (once implemented in later phases) will be considered
successful when:

1. A photographer can go from file upload to a downloadable PDF certificate with working
   Etherscan-equivalent (local block explorer / tx hash) and IPFS links, entirely through the UI.
2. Every minted token has a unique `contentHash`; attempting to mint the same file twice is rejected
   on-chain.
3. `royaltyInfo()` returns the artist-declared royalty split for any token, queryable by any
   EIP-2981-aware marketplace integration.
4. The contract owner can pause minting and transfers, and the artist (token owner) can burn a
   token, without any backend code changes.
5. The full UI, including all user-facing strings, is available in English and Polish via the flag
   switcher, backed by `GET /api/i18n/{lang}`.
6. Hardhat contract tests and backend JUnit tests pass with no live network or Pinata credentials
   required for the unit-test suite.

## 7. Related Documents

- Requirements detail: [02-requirements.md](./02-requirements.md)
- Architecture: [03-architecture.md](./03-architecture.md)
- Full original product brief: `../../requirements.md` (English), `../../requirements_pl.md` (Polish)
