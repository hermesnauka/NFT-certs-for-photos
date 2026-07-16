"""Builds and sends mintCertificate transactions against the deployed PhotoCertificate contract via
web3.py, using an explicit inline ABI (just the three entry points this backend calls) rather than
a full ABI-codegen wrapper — mirrors app01/app03's "explicit ABI encoding" rationale in
../../docs/sdlc/03-architecture.md.
"""

import requests
from eth_account import Account
from web3 import Web3
from web3.exceptions import ContractLogicError, Web3RPCError

from api.errors import ChainUnavailableException, DuplicateContentHashException, MintingException

# Solidity: mintCertificate(address to, string metadataURI, bytes32 contentHash,
#                            address royaltyRecipient, uint256 royaltyFeeBasisPoints)
#           returns (uint256 tokenId)
# Solidity: tokenContentHash(uint256 tokenId) view returns (bytes32)
# Solidity: event CertificateMinted(uint256 indexed tokenId, address indexed to,
#                                    bytes32 contentHash, string metadataURI)
PHOTO_CERTIFICATE_ABI = [
    {
        "type": "function",
        "name": "mintCertificate",
        "stateMutability": "nonpayable",
        "inputs": [
            {"name": "to", "type": "address"},
            {"name": "metadataURI", "type": "string"},
            {"name": "contentHash", "type": "bytes32"},
            {"name": "royaltyRecipient", "type": "address"},
            {"name": "royaltyFeeBasisPoints", "type": "uint256"},
        ],
        "outputs": [{"name": "tokenId", "type": "uint256"}],
    },
    {
        "type": "function",
        "name": "tokenContentHash",
        "stateMutability": "view",
        "inputs": [{"name": "tokenId", "type": "uint256"}],
        "outputs": [{"name": "", "type": "bytes32"}],
    },
    {
        "type": "event",
        "name": "CertificateMinted",
        "anonymous": False,
        "inputs": [
            {"name": "tokenId", "type": "uint256", "indexed": True},
            {"name": "to", "type": "address", "indexed": True},
            {"name": "contentHash", "type": "bytes32", "indexed": False},
            {"name": "metadataURI", "type": "string", "indexed": False},
        ],
    },
]

DUPLICATE_HASH_REVERT_REASON = "PhotoCertificate: duplicate content hash"


class MintResult:
    def __init__(self, token_id, tx_hash, contract_address):
        self.token_id = token_id
        self.tx_hash = tx_hash
        self.contract_address = contract_address


def hex_to_bytes32(hex_str):
    stripped = hex_str[2:] if hex_str.lower().startswith("0x") else hex_str
    if len(stripped) != 64:
        raise MintingException(f"contentHash must be exactly 32 bytes of hex, got {len(stripped)} chars")
    return bytes.fromhex(stripped)


class ContractService:
    """Talks to a real JSON-RPC endpoint. Substituted by a fake in tests (NFR-4) — see
    api/tests/support.py.
    """

    def __init__(self, config):
        self._config = config

    def _web3(self):
        return Web3(Web3.HTTPProvider(self._config.web3_rpc_url))

    def _contract(self, w3):
        return w3.eth.contract(
            address=Web3.to_checksum_address(self._config.nft_contract_address), abi=PHOTO_CERTIFICATE_ABI
        )

    def mint_certificate(self, to, metadata_uri, content_hash_hex, royalty_recipient, royalty_fee_basis_points):
        try:
            account = Account.from_key(self._config.minter_private_key)
        except Exception as exc:
            raise MintingException(f"Minter private key is misconfigured: {exc}")

        content_hash = hex_to_bytes32(content_hash_hex)

        try:
            w3 = self._web3()
            contract = self._contract(w3)
            transaction = contract.functions.mintCertificate(
                Web3.to_checksum_address(to), metadata_uri, content_hash,
                Web3.to_checksum_address(royalty_recipient), royalty_fee_basis_points,
            ).build_transaction({
                "from": account.address,
                "nonce": w3.eth.get_transaction_count(account.address),
                "gas": 4_300_000,  # matches the other ports' DefaultGasProvider-equivalent constant
                "gasPrice": w3.to_wei(20, "gwei"),
            })
            signed = account.sign_transaction(transaction)
            tx_hash = w3.eth.send_raw_transaction(signed.raw_transaction)
            receipt = w3.eth.wait_for_transaction_receipt(tx_hash)
        except (ContractLogicError, Web3RPCError) as exc:
            if DUPLICATE_HASH_REVERT_REASON in str(exc):
                raise DuplicateContentHashException(content_hash_hex)
            raise MintingException(f"Transaction submission failed: {exc}")
        except (requests.RequestException, ConnectionError):
            raise ChainUnavailableException(f"Could not reach Ethereum RPC endpoint: {self._config.web3_rpc_url}")

        if receipt.status != 1:
            raise MintingException(f"mintCertificate transaction reverted (status=0x{receipt.status:x})")

        events = contract.events.CertificateMinted().process_receipt(receipt)
        if not events:
            raise MintingException(
                "mintCertificate succeeded but no CertificateMinted event log was found in the receipt"
            )

        return MintResult(events[0]["args"]["tokenId"], tx_hash.hex(), self._config.nft_contract_address)

    def token_content_hash(self, token_id):
        w3 = self._web3()
        result = self._contract(w3).functions.tokenContentHash(token_id).call()
        if not any(result):
            return None
        return "0x" + result.hex()
