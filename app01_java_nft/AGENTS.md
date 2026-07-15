# app01_java_nft — Reference Implementation

Full product spec: see root `../requirements.md` (`../requirements_pl.md` for Polish) and `docs/sdlc/` in this folder for architecture, API, smart-contract design, threat model, and test plan.

## Layout
- `contracts/` — Hardhat + Solidity (ERC-721 + EIP-2981 + Pausable/Burnable). Local dev chain only, no testnet/mainnet deploy.
- `backend/` — Spring Boot 3 (Java 21), Maven. Hashing, metadata watermarking, Pinata IPFS pinning, mock KYC, Web3j contract calls, PDF certificate generation.
- `frontend/` — Next.js 14 + TypeScript + Tailwind + wagmi/viem. Creator portal: wallet connect, drag-and-drop mint flow, certificate viewer, EN/PL switch.
- `docs/sdlc/` — SDLC documentation set (numbered 01-10).

## Build & test
- Contracts: `cd contracts && npm install && npx hardhat test`
- Backend: `cd backend && mvn test` (unit tests only; use the local fake storage/chain, no live network or Pinata credentials required)
- Frontend: `cd frontend && npm install && npm run build`
- Full local smoke test: see `docs/sdlc/08-test-plan.md`

## Storage provider switch
- `app.storage.provider` (env: `APP_STORAGE_PROVIDER`) = `pinata` (default, real IPFS pinning) or `mock` (local-disk fake, no credentials needed). Active provider is logged at startup.

## Required environment variables (backend, real end-to-end run only)
- `PINATA_JWT` (preferred) or `PINATA_API_KEY` + `PINATA_API_SECRET` — real Pinata pinning, only required when `app.storage.provider=pinata`. The app fails fast if missing in that mode; never commit these.
- `NFT_CONTRACT_ADDRESS`, `WEB3J_RPC_URL` (default `http://localhost:8545` for local Hardhat node), `MINTER_PRIVATE_KEY` (local Hardhat test account only — never a real-funds key).

## Conventions
- Backend package root: `com.devpowers.nftcerts`; Spring Boot conventions (constructor injection, `@Service`/`@RestController` layering, no field injection).
- Solidity: OpenZeppelin contracts only, no hand-rolled token-standard logic.
- Frontend: App Router, server components by default, `"use client"` only where wallet/browser APIs are needed.
- Known simplifications (full ADR in `docs/sdlc/10-glossary-and-decisions.md`): KYC is a mock auto-verifier behind `KycVerificationService`; watermarking is metadata-based (EXIF/XMP), not steganographic; contracts run on a local Hardhat chain only. Storage is **real** Pinata pinning behind `IpfsStorageService` (a local-disk fake implements the same interface for offline unit tests only).
