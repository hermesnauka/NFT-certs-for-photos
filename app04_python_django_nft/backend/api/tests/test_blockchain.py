import pytest

from api.errors import MintingException
from api.services.blockchain import hex_to_bytes32

# ContractService.mint_certificate/token_content_hash themselves require a live JSON-RPC endpoint
# and are exercised by the manual end-to-end smoke test (docs/sdlc/08-test-plan.md §4), consistent
# with NFR-4. The one pure, deterministic piece of logic — validating/parsing the content hash hex
# string — is unit-testable directly.


def test_hex_to_bytes32_valid_hash_with_prefix_returns_32_bytes():
    assert len(hex_to_bytes32("0x" + "a" * 64)) == 32


def test_hex_to_bytes32_valid_hash_without_prefix_returns_32_bytes():
    assert len(hex_to_bytes32("b" * 64)) == 32


@pytest.mark.parametrize("value", ["0x1234", ""])
def test_hex_to_bytes32_wrong_length_raises(value):
    with pytest.raises(MintingException):
        hex_to_bytes32(value)
