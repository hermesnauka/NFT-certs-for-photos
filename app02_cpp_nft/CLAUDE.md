# app02_cpp_nft — C++ Port

C++20 port of the reference implementation in `../app01_java_nft`. Full product spec: root `../requirements.md` (`../requirements_pl.md` for Polish); `docs/sdlc/` in this folder for architecture, API, smart-contract design, threat model, and test plan.

## Layout
- `contracts/` — Hardhat + Solidity (ERC-721 + EIP-2981 + Pausable/Burnable), same design as app01. Local dev chain only.
- `backend/` — C++20, Drogon HTTP framework, CMake. SHA-256 hashing (OpenSSL), EXIF watermarking (Exiv2), Pinata IPFS pinning + local mock, mock KYC, hand-rolled Ethereum stack (Keccak-256, ABI encoding, RLP, secp256k1 legacy-tx signing, JSON-RPC client — no Web3 library), PDF certificates (libharu), SQLite persistence, EN/PL i18n.
- `frontend/` — Next.js 14 + TypeScript + Tailwind + wagmi/viem (same stack as app01).
- `docs/sdlc/` — SDLC documentation set (numbered 01-10).

## Build & test
- Contracts: `cd contracts && npm install && npx hardhat test`
- Backend:
  ```
  cd backend
  cmake -B build -DCMAKE_BUILD_TYPE=Release
  cmake --build build -j$(nproc)
  ctest --test-dir build --output-on-failure
  ```
  System deps: `cmake g++ libdrogon-dev libssl-dev libjsoncpp-dev libsecp256k1-dev libexiv2-dev libsqlite3-dev libhpdf-dev pkg-config`, plus MySQL client headers (`default-libmysqlclient-dev`) which Drogon's CMake config requires even though the app uses SQLite. Without root, download/extract the headers locally (`apt-get download libmariadb-dev && dpkg -x ... <dir>`) and configure with `-DMARIADB_INCLUDE_DIRS=<dir>/usr/include/mariadb -DMYSQL_LIBRARIES=/usr/lib/x86_64-linux-gnu/libmariadb.so.3`. GoogleTest is fetched via FetchContent on first configure (needs network once).
- Tests are offline unit tests: in-memory SQLite, local stub storage, no live chain or Pinata credentials. Blockchain test vectors (calldata, signed raw tx, hashes) are ethers-v6 ground truth — regenerate with ethers from `contracts/node_modules`, never hand-edit hex literals.
- Frontend: `cd frontend && npm install && npm run build`

## Run (backend)
`./build/nft_certs_backend` — listens on `SERVER_PORT` (default 8081), SQLite at `DB_PATH` (default `data/nft-certs.db`).

## Environment variables
- `APP_STORAGE_PROVIDER` = `pinata` (default, real IPFS pinning) or `mock` (local-disk fake, no credentials). Active provider is logged at startup; the app fails fast if Pinata mode lacks credentials.
- `PINATA_JWT` (preferred) or `PINATA_API_KEY` + `PINATA_API_SECRET` — never commit these.
- `NFT_CONTRACT_ADDRESS`, `MINTER_PRIVATE_KEY` (local Hardhat test account only — never a real-funds key), `WEB3J_RPC_URL` (default `http://localhost:8545`).

## Conventions
- Namespace root `nftcerts::` with one sub-namespace per module (`api`, `blockchain`, `certificate`, `db`, `identity`, `storage`, ...); constructor dependency injection via references, interfaces as abstract base classes (`IpfsStorageService`, `KycVerificationService`).
- Same known simplifications as app01 (ADR in `docs/sdlc/10-glossary-and-decisions.md`): mock KYC, metadata-based watermarking, local Hardhat chain only, real Pinata pinning behind an interface with a local stub for tests.
