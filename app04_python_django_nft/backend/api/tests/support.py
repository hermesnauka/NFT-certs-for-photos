"""Test doubles standing in for a live chain (NFR-4: automated tests must not require live network
access). Mirrors app03's FakeContractService.
"""

from api.errors import DuplicateContentHashException
from api.services.blockchain import MintResult


class FakeContractService:
    def __init__(self):
        self._next_token_id = 1
        self.registered_hashes = set()
        self.mint_override = None

    def mint_certificate(self, to, metadata_uri, content_hash_hex, royalty_recipient, royalty_fee_basis_points):
        if self.mint_override is not None:
            return self.mint_override(to, metadata_uri, content_hash_hex, royalty_recipient,
                                        royalty_fee_basis_points)

        if content_hash_hex in self.registered_hashes:
            raise DuplicateContentHashException(content_hash_hex)
        self.registered_hashes.add(content_hash_hex)

        token_id = self._next_token_id
        self._next_token_id += 1
        return MintResult(token_id, "0x" + "aa" * 32, "0x" + "bb" * 20)

    def token_content_hash(self, token_id):
        return None
