# app04_python_django_nft — Python/Django Port

Python/Django port of the reference implementation in `../app01_java_nft`. Full product spec: root `../requirements.md` (`../requirements_pl.md` for Polish); `docs/sdlc/` in this folder for architecture, API, smart-contract design, threat model, and test plan.

## Layout
- `contracts/` — Hardhat + Solidity (ERC-721 + EIP-2981 + Pausable/Burnable), same design as app01. Local dev chain only.
- `backend/` — Django 6 + Django REST Framework (Python 3), SQLite via the Django ORM. SHA-256 hashing (`hashlib`), EXIF watermarking (Pillow + piexif), Pinata IPFS pinning + local stub, mock KYC, web3.py for chain calls (an explicit inline ABI covering just the three entry points the backend calls, rather than a full ABI-codegen wrapper — mirrors app01's "explicit ABI encoding" rationale), PDF certificates (reportlab), EN/PL i18n message maps.
- `frontend/` — Next.js 14 + TypeScript + Tailwind + wagmi/viem (same stack as app01).
- `docs/sdlc/` — SDLC documentation set (numbered 01-10).

## Build & test
- Contracts: `cd contracts && npm install && npx hardhat test`
- Backend:
  ```
  cd backend
  python3 -m venv .venv && source .venv/bin/activate
  pip install -r requirements.txt
  python manage.py migrate
  python -m pytest              # 58 tests: unit tests per service + full HTTP integration tests via DRF's APIClient
  python manage.py runserver 0.0.0.0:8000    # real run: requires env vars below to be set
  ```
- Tests are offline: every test runs against `LocalStubIpfsStorage` and a `FakeContractService` (see `conftest.py`'s `fake_external_services` autouse fixture, `api/tests/support.py`) instead of real Pinata/chain calls, and Django's isolated per-run test database. No live network or credentials required (NFR-4).
- Frontend: `cd frontend && npm install && npm run build`

## Storage provider switch
- `APP_STORAGE_PROVIDER` = `pinata` (default, real IPFS pinning) or `mock` (local-disk fake, no credentials needed). Active provider is logged at startup (`nftcerts/settings.py`'s `PlatformConfig`); the switch is applied when `api/registry.py` builds the process-wide `IpfsStorageService` singleton.

## Required environment variables (backend, real end-to-end run only)
- `PINATA_JWT` (preferred) or `PINATA_API_KEY` + `PINATA_API_SECRET` — real Pinata pinning, only required when `APP_STORAGE_PROVIDER=pinata`. The app fails fast (`PlatformConfig.validate()`) if missing in that mode; never commit these.
- `NFT_CONTRACT_ADDRESS`, `WEB3J_RPC_URL` (default `http://localhost:8545` for local Hardhat node), `MINTER_PRIVATE_KEY` (local Hardhat test account only — never a real-funds key).
- `SERVER_PORT` (default `8000`, informational — `manage.py runserver` still takes the bind address as a positional argument).
- All of the above are also loadable from a gitignored `.env` (or `../.env`) file; real environment variables always take precedence.

## Conventions
- Backend composition root: `api/registry.py` builds the process-wide service singletons (`storage_service`, `kyc_service`, `contract_service`, `certificate_service`), reading the storage-provider switch from `django.conf.settings.APP_CONFIG`. Views (`api/views.py`) stay thin — parse the request, delegate to `api.registry.certificate_service` / `api.registry.kyc_service`, return a DRF `Response`.
- One view class per endpoint (`api/views.py`), matching the controller-per-concern layout of the other three ports. Business logic lives in `api/certificate_service.py`, not the views.
- One exception class per error case (`api/errors.py`), each carrying its HTTP status; `api.errors.exception_handler` (wired via `REST_FRAMEWORK["EXCEPTION_HANDLER"]`) renders *every* exception — not just these — into the shared `{timestamp, status, error, message, path}` shape.
- Solidity: OpenZeppelin contracts only, no hand-rolled token-standard logic.
- Frontend: App Router, server components by default, `"use client"` only where wallet/browser APIs are needed.
- Known simplifications (full ADR in `docs/sdlc/10-glossary-and-decisions.md`): KYC is a mock auto-verifier behind `KycVerificationService`; watermarking is metadata-based (EXIF), not steganographic; contracts run on a local Hardhat chain only. Storage is **real** Pinata pinning behind `IpfsStorageService` (a local-disk stub implements the same interface for offline unit tests only).
