# 06 — Smart Contract Design

Contract: `contracts/contracts/PhotoCertificate.sol`. Built with Hardhat and OpenZeppelin,
deployed to a **local Hardhat network only** for this phase (see
[09-deployment-and-devops.md](./09-deployment-and-devops.md)).

## 1. Inheritance Chain

```solidity
contract PhotoCertificate is
    ERC721,
    ERC2981,
    Pausable,
    ERC721Burnable,
    AccessControl
{ ... }
```

| Base contract | Standard | Why it's included |
|----------------|----------|---------------------|
| `ERC721` | ERC-721 | Core non-fungible token standard: ownership, transfer, approval. Chosen over ERC-1155 because each photograph is a unique, non-fungible certificate (one artwork = one token), not a multi-edition/fungible-class item. |
| `ERC2981` | EIP-2981 | Standardized royalty-info interface (`royaltyInfo`) so any compliant marketplace (OpenSea, Rarible, Blur) can honor secondary-sale royalties without a marketplace-specific side contract. |
| `Pausable` | OpenZeppelin utility | Circuit breaker: lets `PAUSER_ROLE` halt minting and transfers in response to a legal copyright claim, without altering ownership records. |
| `ERC721Burnable` | OpenZeppelin ERC-721 extension | Lets a token owner permanently destroy their certificate (e.g., a settled dispute or a withdrawn work), preserving the owner's unilateral right to exit rather than requiring operator intervention. |
| `AccessControl` | OpenZeppelin utility | Role-based permissioning for `MINTER_ROLE` and `PAUSER_ROLE`, instead of a single `Ownable` address, since minting and pausing are operationally distinct responsibilities. |

Function resolution note: because both `ERC721` and `Pausable`/`ERC721Burnable` participate in
transfer hooks, `_update` (OZ v5 pattern) is overridden once in `PhotoCertificate` to combine the
pausable check with the base ERC-721 transfer logic; `supportsInterface` is overridden to combine
`ERC721`, `ERC2981`, and `AccessControl` interface IDs.

## 2. Roles

| Role | Constant | Assigned to (this phase) | Controls |
|------|----------|------------------------------|----------|
| Default admin | `DEFAULT_ADMIN_ROLE` | Deployer account | Grant/revoke `MINTER_ROLE` and `PAUSER_ROLE` |
| Minter | `MINTER_ROLE` | Backend's `ContractService` account (`MINTER_PRIVATE_KEY`) | Call `mintCertificate` |
| Pauser | `PAUSER_ROLE` | Deployer / platform operator account | Call `pause()` / `unpause()` |

Roles are granted in the constructor/deploy script to the deployer by default, with `MINTER_ROLE`
additionally granted to the backend's configured account address at deploy time.

## 3. Core Function: `mintCertificate`

```solidity
function mintCertificate(
    address to,
    string calldata metadataURI,
    bytes32 contentHash,
    address royaltyRecipient,
    uint256 royaltyFeeBasisPoints
)
    external
    onlyRole(MINTER_ROLE)
    whenNotPaused
    returns (uint256 tokenId)
{
    require(!hashRegistered[contentHash], "PhotoCertificate: duplicate content hash");

    tokenId = _nextTokenId++;
    hashRegistered[contentHash] = true;
    _tokenContentHash[tokenId] = contentHash;

    _safeMint(to, tokenId);
    _setTokenURI(tokenId, metadataURI);
    _setTokenRoyalty(tokenId, royaltyRecipient, uint96(royaltyFeeBasisPoints));

    emit CertificateMinted(tokenId, to, contentHash, metadataURI);
}
```

Behavioral notes:

- **Access-gated**: only an address holding `MINTER_ROLE` may call this (the backend's
  `ContractService`, never a user's own wallet in this phase — see
  [03-architecture.md](./03-architecture.md) trust boundaries).
- **Pause-gated**: reverts if the contract is paused (`whenNotPaused`), so a legal hold stops new
  certificates from being issued as well as blocking transfers.
- **Duplicate-hash protection**: `hashRegistered[contentHash]` is checked before any state is
  written; a second mint attempt for the same photograph (same SHA-256 hash) reverts. This is the
  on-chain enforcement mirrored by the API's `409` response in
  [05-api-design.md](./05-api-design.md).
- **Royalty set at mint time**: `_setTokenRoyalty` (from `ERC2981`) stores `royaltyRecipient` and
  `royaltyFeeBasisPoints` (as a fraction out of 10,000, OpenZeppelin's `_feeDenominator()`
  convention) per-token, so each certificate can have an independent royalty split.

### Duplicate-Hash Protection Rationale

The core authenticity promise of the platform (see
[01-vision-and-scope.md](./01-vision-and-scope.md), G1) is that a `contentHash` uniquely and
permanently identifies one certificate. Without the `hashRegistered` mapping, nothing on-chain
would stop the same file (or a byte-identical re-upload) from being minted twice — by the same
artist accidentally, or, more importantly, by anyone who obtains the file and convinces the
backend's KYC/minter flow to process it again. Rejecting at the contract layer (not just the
backend/API layer) means the guarantee holds even if the backend's own duplicate check is ever
bypassed, misconfigured, or reimplemented incorrectly — the chain is the final authority.

## 4. Supporting View Functions

```solidity
function tokenContentHash(uint256 tokenId) external view returns (bytes32);

function royaltyInfo(uint256 tokenId, uint256 salePrice)
    external
    view
    returns (address receiver, uint256 royaltyAmount); // standard EIP-2981 signature
```

- `tokenContentHash` lets any client (backend, block explorer, marketplace tooling) verify that a
  given token's registered hash matches the hash of a candidate file — the on-chain half of the
  authenticity check.
- `royaltyInfo` is the standard EIP-2981 read function; marketplaces call this automatically when
  computing secondary-sale payouts. No custom interface is needed for marketplace compatibility.

## 5. Pause / Burn Mechanics for Copyright Disputes

| Function | Role required | Effect |
|----------|-----------------|--------|
| `pause()` | `PAUSER_ROLE` | Sets `Pausable._paused = true`. Blocks `mintCertificate` (via `whenNotPaused`) and blocks transfers (via the overridden `_update` hook checking `whenNotPaused`). Existing token data and ownership records are untouched. |
| `unpause()` | `PAUSER_ROLE` | Reverses the above once a dispute is resolved. |
| `burn(uint256 tokenId)` (from `ERC721Burnable`) | Token owner or approved operator only (standard OZ behavior — **not** role-gated) | Permanently destroys the token. `hashRegistered[contentHash]` is deliberately **not** cleared on burn, so a burned (disputed) photograph's hash can never be re-minted under a different claimant — burn removes the certificate from circulation without reopening the duplicate-protection guarantee. |

Design intent: `pause` is a **platform-level, time-boxed circuit breaker** (e.g., "a court order is
pending, freeze all activity"), while `burn` is an **individual, owner-initiated, permanent**
action (e.g., "this specific certificate is being withdrawn"). Neither mechanism grants the
platform operator the ability to seize or reassign a token — ownership transfer still requires the
current owner's (or an approved operator's) action, preserving the decentralization principle
required by the spec even while giving the operator a legal-compliance safety valve.

## 6. Events

```solidity
event CertificateMinted(
    uint256 indexed tokenId,
    address indexed to,
    bytes32 contentHash,
    string metadataURI
);
```

Emitted on every successful mint; indexed on `tokenId` and `to` for efficient off-chain filtering
(e.g., the backend could reconcile its `Certificate` table against chain events, though this phase
relies on synchronous confirmation from the mint transaction receipt rather than an event
indexer). Standard `Transfer`, `Paused`, `Unpaused`, `RoleGranted`/`RoleRevoked` events are emitted
automatically by the inherited OpenZeppelin contracts.

## 7. Gas Considerations

- `mintCertificate` performs a bounded, constant number of storage writes per call: one
  `hashRegistered` write, one `_tokenContentHash` write, the `ERC721` ownership writes performed by
  `_safeMint`, one token URI write, and one royalty-info write. No loops, no unbounded arrays — gas
  cost is predictable and does not grow with the number of previously minted tokens.
- `hashRegistered` uses `bytes32` (a single storage slot) rather than a `string`/`bytes` mapping,
  keeping the duplicate check cheap (`SLOAD`) rather than requiring hashing or comparison of
  variable-length data on-chain (the hash is already computed off-chain by the backend's
  `HashingService`).
- `_safeMint` (vs. `_mint`) incurs the extra cost of an `onERC721Received` check when minting to a
  contract address; this is accepted for safety since the recipient is an artist-controlled wallet
  address that may in principle be a smart-contract wallet.
- Local Hardhat network has no real gas cost for this phase, but the design intentionally avoids
  patterns (unbounded loops, per-call dynamic array growth) that would be expensive if later
  promoted to a public testnet/mainnet (see
  [09-deployment-and-devops.md](./09-deployment-and-devops.md) promotion path).

## 8. Planned Hardhat/Chai Test Cases

See [08-test-plan.md](./08-test-plan.md) for the full test strategy; the contract-level cases are
enumerated there and include: successful mint, royalty math via `royaltyInfo`, pause blocking both
mint and transfer, burn removing the token while keeping `hashRegistered` set, access-control
rejection of non-minter/non-pauser callers, and duplicate-hash rejection.

## Related Documents

- [04-data-model.md](./04-data-model.md) — on-chain storage layout
- [05-api-design.md](./05-api-design.md) — how the backend calls `mintCertificate`
- [07-security-and-threat-model.md](./07-security-and-threat-model.md) — minter key custody, replay/duplicate threats
- [08-test-plan.md](./08-test-plan.md) — full contract test list
