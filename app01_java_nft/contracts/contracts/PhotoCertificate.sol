// SPDX-License-Identifier: MIT
pragma solidity ^0.8.24;

import {ERC721} from "@openzeppelin/contracts/token/ERC721/ERC721.sol";
import {ERC721URIStorage} from "@openzeppelin/contracts/token/ERC721/extensions/ERC721URIStorage.sol";
import {ERC721Burnable} from "@openzeppelin/contracts/token/ERC721/extensions/ERC721Burnable.sol";
import {ERC2981} from "@openzeppelin/contracts/token/common/ERC2981.sol";
import {Pausable} from "@openzeppelin/contracts/utils/Pausable.sol";
import {AccessControl} from "@openzeppelin/contracts/access/AccessControl.sol";

/**
 * @title PhotoCertificate
 * @notice ERC-721 NFT certificate-of-authenticity contract for fine-art photography.
 *
 * @dev Design reference: docs/sdlc/06-smart-contract-design.md (authoritative design doc),
 * docs/sdlc/08-test-plan.md (contract test case list), docs/sdlc/04-data-model.md (on-chain
 * storage layout).
 *
 * Correction vs. the design doc: doc 06's inheritance list omits `ERC721URIStorage`, but its
 * `mintCertificate` snippet calls `_setTokenURI`, which only exists on `ERC721URIStorage` in
 * OpenZeppelin v5 (base `ERC721` has no per-token URI storage or `_setTokenURI` in v5). This
 * contract adds `ERC721URIStorage` to the inheritance chain so the design compiles and behaves
 * as documented.
 *
 * Each photograph is a unique, non-fungible certificate (one artwork = one token), so ERC-721
 * was chosen over ERC-1155 per doc 06 section 1.
 */
contract PhotoCertificate is
    ERC721,
    ERC721URIStorage,
    ERC2981,
    Pausable,
    ERC721Burnable,
    AccessControl
{
    /// @notice Role allowed to call {mintCertificate}. Granted to the backend's ContractService
    /// account (see docs/sdlc/06-smart-contract-design.md section 2).
    bytes32 public constant MINTER_ROLE = keccak256("MINTER_ROLE");

    /// @notice Role allowed to call {pause}/{unpause}. Granted to the deployer / platform
    /// operator account (see docs/sdlc/06-smart-contract-design.md section 2).
    bytes32 public constant PAUSER_ROLE = keccak256("PAUSER_ROLE");

    /// @dev Monotonically increasing token ID counter. Starts at 1 so token ID 0 is never issued
    /// (keeps "unset"/"nonexistent" checks unambiguous downstream).
    uint256 private _nextTokenId = 1;

    /// @notice Tracks every content hash (SHA-256 of the source image) that has ever been minted,
    /// so the same photograph can never be certified twice — even after the resulting token is
    /// burned (see docs/sdlc/06-smart-contract-design.md section 5).
    mapping(bytes32 => bool) public hashRegistered;

    /// @dev Per-token content hash, exposed publicly via {tokenContentHash}.
    mapping(uint256 => bytes32) private _tokenContentHash;

    /// @notice Emitted on every successful mint; indexed on `tokenId` and `to` for efficient
    /// off-chain filtering/reconciliation.
    event CertificateMinted(
        uint256 indexed tokenId,
        address indexed to,
        bytes32 contentHash,
        string metadataURI
    );

    /**
     * @param name_ ERC-721 collection name (e.g. "Fine Art Photo Certificate").
     * @param symbol_ ERC-721 collection symbol (e.g. "PHOTOCERT").
     *
     * Grants `DEFAULT_ADMIN_ROLE`, `MINTER_ROLE`, and `PAUSER_ROLE` to the deployer. The deploy
     * script may additionally grant `MINTER_ROLE` to a configurable backend account.
     */
    constructor(string memory name_, string memory symbol_) ERC721(name_, symbol_) {
        _grantRole(DEFAULT_ADMIN_ROLE, msg.sender);
        _grantRole(MINTER_ROLE, msg.sender);
        _grantRole(PAUSER_ROLE, msg.sender);
    }

    /**
     * @notice Mints a new NFT certificate for a photograph, registers its content hash, sets its
     * metadata URI, and configures per-token EIP-2981 royalty info.
     *
     * @param to Recipient of the minted certificate (the artist's wallet address).
     * @param metadataURI IPFS/Arweave URI of the metadata JSON describing the artwork.
     * @param contentHash SHA-256 hash of the source image file, computed off-chain.
     * @param royaltyRecipient Address that will receive EIP-2981 secondary-sale royalties.
     * @param royaltyFeeBasisPoints Royalty rate in basis points (out of 10,000).
     * @return tokenId The newly minted token's ID.
     *
     * Reverts if the caller lacks `MINTER_ROLE`, if the contract is paused, or if `contentHash`
     * has already been registered by a previous mint (duplicate-content protection — see
     * docs/sdlc/06-smart-contract-design.md section 3).
     */
    function mintCertificate(
        address to,
        string calldata metadataURI,
        bytes32 contentHash,
        address royaltyRecipient,
        uint256 royaltyFeeBasisPoints
    ) external onlyRole(MINTER_ROLE) whenNotPaused returns (uint256 tokenId) {
        require(!hashRegistered[contentHash], "PhotoCertificate: duplicate content hash");

        tokenId = _nextTokenId++;
        hashRegistered[contentHash] = true;
        _tokenContentHash[tokenId] = contentHash;

        _safeMint(to, tokenId);
        _setTokenURI(tokenId, metadataURI);
        _setTokenRoyalty(tokenId, royaltyRecipient, uint96(royaltyFeeBasisPoints));

        emit CertificateMinted(tokenId, to, contentHash, metadataURI);
    }

    /**
     * @notice Returns the registered SHA-256 content hash for a given token.
     * @dev Reverts with a clear message if the token does not exist.
     */
    function tokenContentHash(uint256 tokenId) external view returns (bytes32) {
        require(_ownerOf(tokenId) != address(0), "PhotoCertificate: token does not exist");
        return _tokenContentHash[tokenId];
    }

    /**
     * @notice Pauses minting and transfers. Intended as a platform-level, time-boxed circuit
     * breaker in response to a legal copyright claim (see docs/sdlc/06-smart-contract-design.md
     * section 5). Does not alter ownership records.
     */
    function pause() external onlyRole(PAUSER_ROLE) {
        _pause();
    }

    /// @notice Reverses {pause} once a dispute is resolved.
    function unpause() external onlyRole(PAUSER_ROLE) {
        _unpause();
    }

    // ---------------------------------------------------------------------
    // Required overrides for OpenZeppelin v5 multiple inheritance
    // ---------------------------------------------------------------------

    /**
     * @dev Combines the base ERC-721 transfer hook with a pause check. `ERC721URIStorage` in this
     * OpenZeppelin version does not itself override `_update` (it only overrides `tokenURI` and
     * `supportsInterface`), so only `ERC721` needs to appear in this override list.
     * `mintCertificate` already enforces `whenNotPaused` at the entry point, but this hook is
     * what also blocks direct transfers (`transferFrom`/`safeTransferFrom`) and burns while
     * paused, since `Pausable` does not automatically hook into `ERC721` in OZ v5.
     */
    function _update(
        address to,
        uint256 tokenId,
        address auth
    ) internal override(ERC721) returns (address) {
        require(!paused(), "Pausable: paused");
        return super._update(to, tokenId, auth);
    }

    function tokenURI(
        uint256 tokenId
    ) public view override(ERC721, ERC721URIStorage) returns (string memory) {
        return super.tokenURI(tokenId);
    }

    function supportsInterface(
        bytes4 interfaceId
    )
        public
        view
        override(ERC721, ERC721URIStorage, ERC2981, AccessControl)
        returns (bool)
    {
        return super.supportsInterface(interfaceId);
    }
}
