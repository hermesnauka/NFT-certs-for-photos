from api.models import Artwork, Certificate
from api.services.pdf import generate


def test_generate_returns_non_empty_valid_pdf_bytes():
    artwork = Artwork(
        id="11111111-1111-1111-1111-111111111111",
        file_id="22222222-2222-2222-2222-222222222222",
        original_filename="photo.jpg",
        sha256_hash="9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a0",
        title="Lighthouse at Dusk",
        artist_did="did:key:z6Mk...",
        medium="Archival pigment print",
        year_created=2024,
        royalty_percentage_bps=750,
        artist_wallet_address="0x" + "a" * 40,
        image_ipfs_uri="ipfs://image-cid",
        metadata_ipfs_uri="ipfs://metadata-cid",
        created_at="2026-07-15T10:15:30Z",
    )
    certificate = Certificate(
        token_id=42,
        artwork=artwork,
        content_hash_hex=artwork.sha256_hash,
        contract_address="0x" + "b" * 40,
        tx_hash="0xdeadbeef",
        owner_address="0x" + "a" * 40,
        royalty_percentage_bps=750,
        royalty_recipient="0x" + "a" * 40,
        minted_at="2026-07-15T10:15:30Z",
    )

    pdf_bytes = generate(certificate, "http://etherscan/tx/0xdeadbeef", "http://opensea/x/42",
                          "http://rarible/x:42")

    assert len(pdf_bytes) > 0
    assert pdf_bytes[:5] == b"%PDF-"
    assert b"%%EOF" in pdf_bytes
