
# ROLE 1: ACT AS THE LEAD WEB3 & SMART CONTRACT ARCHITECT

## GOAL
Design and implement the backend architecture and Smart Contracts for a compliant art tokenization platform (fine art photography). The system must ingest high-quality images, generate cryptographic proof of authenticity for them, and then mint NFT certificates prepared for sale on public marketplaces (e.g., OpenSea, Rarible, Blur), with an absolute emphasis on protecting the creator's copyright.
The application must have a switch to display it in Polish or English, using a simple switcher with a flag.

## IMPLEMENTATION REQUIREMENTS:
1. **Digital Content Protection & Authenticity:**
   - Implement a cryptographic hashing mechanism for image files (e.g., SHA-256) prior to uploading them to decentralized storage.
   - Develop a process of "digital watermarking" or steganography combined with metadata to indisputably link the file to the artist's identity.
2. **Decentralized Storage (IPFS / Arweave):**
   - Configure connectors for permanent, distributed storage of source files and metadata (JSON) on IPFS or Arweave, guaranteeing the immutability of the work (immutable URLs).
3. **Smart Contracts (Solidity / Java Web3j):**
   - Implement and optimize contracts in the ERC-721 or ERC-1155 standards.
   - Implement the **EIP-2981 (NFT Royalty Standard)** to legally and technically secure secondary sale royalties for creators across all public marketplaces.
   - Add pause (Pausable) or burn (Burnable) mechanisms in case of legal copyright claims while maintaining the principle of decentralization.
4. **Creator Identity (DID and KYC):**
   - Design the artist identity verification flow (Decentralized Identifiers - DID or KYC provider integration) to confirm that the minting entity is the rightful copyright holder of the photograph.

## FORMAT & CONNECTORS
List the necessary connectors (e.g., IPFS HTTP Client, Web3j / Ethers.js, Pinata API). Provide complete Smart Contract code along with extensive unit tests. Use professional engineering terminology.
The application must have a switch to display it in Polish or English, using a simple switcher with a flag.


#


# ROLE 2: ACT AS THE LEAD DAPP FRONTEND & UX/UI ENGINEER

## GOAL
Build an intuitive, secure, and professional creator portal (Creator Portal - Frontend in React/Next.js). The application will serve as a secure gateway for uploading art photographs, generating NFT certificates for them, and managing their sales on public marketplaces. The frontend must educate creators about their rights and transparently display the process of safeguarding their work.
The application must have a switch to display it in Polish or English, using a simple switcher with a flag.

## FRONTEND FUNCTIONAL REQUIREMENTS:
1. **Wallet Management (Web3 Integration):**
   - Implement secure wallet connection (e.g., using WalletConnect, MetaMask, Coinbase Wallet).
   - Create a clear Dashboard displaying the artist's assets, earned royalties, and ownership history of generated certificates.
2. **Certificate Creator (Minting Interface):**
   - Design a "Drag & Drop" image upload form that generates and displays the file hash (e.g., SHA-256) to the user in real-time, educating them that this is the "digital fingerprint" protecting their work.
   - Allow metadata definition: Title, Description, Medium, Year of Creation, Royalty Percentage.
3. **Security and Copyright Visualization:**
   - Display the generated digital certificate of ownership as an elegant, downloadable (e.g., PDF) document containing links to transactions on the blockchain (Etherscan) and IPFS links.
   - Add an educational module explaining in simple terms to the artist how the Smart Contract guarantees authenticity and protects against piracy on external marketplaces.
4. **Marketplace Integration (Public Marketplaces):**
   - Implement APIs or deep linking that allows generating direct buttons ("List on OpenSea", "View on Rarible") after a successful minting process, verifying the contract's presence on those platforms.

## EXPECTED OUTCOME
Provide the component tree architecture (e.g., based on tailwindcss and wagmi/viem for React). Write code for key components responsible for the transaction signing process and uploading files to IPFS. Maintain strict client-side data security.
The application must have a switch to display it in Polish or English, using a simple switcher with a flag.
