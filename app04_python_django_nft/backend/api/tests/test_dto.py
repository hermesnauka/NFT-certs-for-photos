from django.conf import settings

from api.dto import build_etherscan_url, build_opensea_url, build_rarible_url, to_dto
from api.models import Artwork, Certificate


def _sample_certificate():
    artwork = Artwork(
        id="artwork-1", file_id="file-1", original_filename="photo.jpg",
        sha256_hash="9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a0",
        title="Lighthouse at Dusk", royalty_percentage_bps=750,
        artist_wallet_address="0x" + "a" * 40, image_ipfs_uri="ipfs://img",
        metadata_ipfs_uri="ipfs://meta", created_at="2026-01-01T00:00:00Z",
    )
    return Certificate(
        token_id=42, artwork=artwork, content_hash_hex=artwork.sha256_hash,
        contract_address="0x" + "c" * 40, tx_hash="0xdeadbeef",
        owner_address="0x" + "a" * 40, royalty_percentage_bps=750, minted_at="2026-01-01T00:00:00Z",
    )


def test_build_etherscan_url_appends_tx_hash_to_base():
    certificate = _sample_certificate()
    assert build_etherscan_url(certificate) == settings.APP_CONFIG.etherscan_base_url + "0xdeadbeef"


def test_build_opensea_url_appends_contract_and_token_id():
    certificate = _sample_certificate()
    expected = f"{settings.APP_CONFIG.opensea_base_url}{certificate.contract_address}/42"
    assert build_opensea_url(certificate) == expected


def test_build_rarible_url_appends_contract_colon_token_id():
    certificate = _sample_certificate()
    expected = f"{settings.APP_CONFIG.rarible_base_url}{certificate.contract_address}:42"
    assert build_rarible_url(certificate) == expected


def test_to_dto_includes_all_documented_fields():
    dto = to_dto(_sample_certificate())

    assert dto["tokenId"] == 42
    assert dto["artworkId"] == "artwork-1"
    assert dto["title"] == "Lighthouse at Dusk"
    assert dto["royaltyPercentageBps"] == 750
    assert dto["imageIpfsUri"] == "ipfs://img"
    assert dto["metadataIpfsUri"] == "ipfs://meta"
    assert "etherscanUrl" in dto
    assert "openSeaUrl" in dto
    assert "raribleUrl" in dto
