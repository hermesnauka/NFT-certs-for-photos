# app02_cpp_nft — NFT Certificates for Photos (C++ port)

C++20 port of the Java reference implementation in `../app01_java_nft`: a compliant NFT certification platform for fine-art photography (SHA-256 proof of authenticity, IPFS pinning via Pinata, ERC-721 + EIP-2981 royalties, mock DID/KYC verification, EN/PL UI). Full spec: `requirements.md` / `requirements_pl.md`; design docs in `docs/sdlc/`.

Stack: Drogon (HTTP) + OpenSSL + Exiv2 + libharu + SQLite backend, with a hand-rolled Ethereum layer (Keccak-256, ABI encoding, RLP, secp256k1 signing, JSON-RPC) instead of a Web3 library; Hardhat/Solidity contracts; Next.js 14 frontend.

## Quick start

> **Secrets:** keep credentials in a gitignored `.env` or real environment variables (or a vault) — never commit `PINATA_JWT`/`PINATA_API_*` or any private key. `MINTER_PRIVATE_KEY` must only ever be a local Hardhat test-account key.

1. **Contracts** — `cd contracts && npm install && npx hardhat node` (terminal 1), then `npx hardhat run scripts/deploy.js --network localhost` (note the deployed address).
2. **Backend** — see `CLAUDE.md` for system packages, then:
   ```
   cd backend
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build -j$(nproc)
   ctest --test-dir build --output-on-failure   # offline unit tests
   APP_STORAGE_PROVIDER=mock NFT_CONTRACT_ADDRESS=<deployed> \
     MINTER_PRIVATE_KEY=<hardhat account #0 key> ./build/nft_certs_backend
   ```
   Set `APP_STORAGE_PROVIDER=pinata` plus `PINATA_JWT` for real IPFS pinning.
3. **Frontend** — `cd frontend && npm install && npm run dev`, then open http://localhost:3000.
