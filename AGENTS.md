# NFT Certificates for Photos

Monorepo for a compliant NFT certification platform for fine-art photography. Full product spec lives in `requirements.md` (English) / `requirements_pl.md` (Polish): cryptographic proof of authenticity, IPFS pinning via Pinata, ERC-721 + EIP-2981 royalties, DID/KYC creator verification, EN/PL UI switch.

## Layout
- `app01_java_nft/` — reference implementation (Java/Spring Boot backend + Solidity contracts + Next.js frontend). See its own `AGENTS.md`/`CLAUDE.md` for build/test commands and `docs/sdlc/` for architecture and design docs.
- `app02_cpp_nft/` — C++20/Drogon port of the same product.
- `app03_csharp_nft/` — C#/.NET port of the same product.
- `app04_python_django_nft/` — Python/Django port of the same product.

Each port shares the same product spec, API contract, data model, and Solidity contract as app01 (see that port's `docs/sdlc/`); only each port's own `AGENTS.md`/`CLAUDE.md` documents its actual tech stack and build/test commands.

## Conventions for every app in this repo
- Keep secrets out of git: use a gitignored `.env` or real environment variables, never hardcode API keys or private keys. Pinata credentials (`PINATA_JWT` or `PINATA_API_KEY`/`PINATA_API_SECRET`) must never be committed.
- Each app subfolder owns its own `AGENTS.md`/`CLAUDE.md` with build/test/run commands specific to that app — don't re-paste the product spec into it; link back to `requirements.md` instead.
