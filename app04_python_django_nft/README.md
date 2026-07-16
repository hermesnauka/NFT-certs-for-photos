# app04_python_django_nft — NFT Certificates for Photos (Python/Django port)

Python/Django port of the Java reference implementation in `../app01_java_nft`: a compliant NFT certification platform for fine-art photography (SHA-256 proof of authenticity, IPFS pinning via Pinata, ERC-721 + EIP-2981 royalties, mock DID/KYC verification, EN/PL UI). Full spec: `requirements.md` / `requirements_pl.md`; design docs in `docs/sdlc/`.

Stack: Django 6 + Django REST Framework, SQLite via the Django ORM, Pillow + piexif for EXIF watermarking, web3.py + eth-account for chain calls, reportlab for PDF certificates; Hardhat/Solidity contracts; Next.js 14 frontend.

## Quick start

> **Secrets:** keep credentials in a gitignored `.env` or real environment variables (or a vault) — never commit `PINATA_JWT`/`PINATA_API_*` or any private key. `MINTER_PRIVATE_KEY` must only ever be a local Hardhat test-account key.

1. **Contracts** — `cd contracts && npm install && npx hardhat node` (terminal 1), then `npx hardhat run scripts/deploy.js --network localhost` (note the deployed address).
2. **Backend** — see `CLAUDE.md` for the full test/env-var reference, then:
   ```
   cd backend
   python3 -m venv .venv && source .venv/bin/activate
   pip install -r requirements.txt
   python manage.py migrate
   python -m pytest      # offline unit + HTTP integration tests
   APP_STORAGE_PROVIDER=mock NFT_CONTRACT_ADDRESS=<deployed> \
     MINTER_PRIVATE_KEY=<hardhat account #0 key> python manage.py runserver 0.0.0.0:8000
   ```
   Set `APP_STORAGE_PROVIDER=pinata` plus `PINATA_JWT` for real IPFS pinning.
3. **Frontend** — `cd frontend && npm install && npm run dev`, then open http://localhost:3001.
