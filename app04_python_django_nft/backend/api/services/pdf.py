"""Renders a one-page, downloadable digital certificate of authenticity for a minted NFT
certificate, via reportlab (pure Python, no headless-browser dependency — see NFR-13). app01 used
OpenPDF, app02 libharu, app03 QuestPDF.
"""

import io

from reportlab.lib.pagesizes import A4
from reportlab.lib.units import mm
from reportlab.pdfgen import canvas


def _dash_if_empty(value):
    return value if value not in (None, "") else "-"


def generate(certificate, etherscan_url, opensea_url, rarible_url):
    artwork = certificate.artwork
    rows = [
        ("Token ID", str(certificate.token_id)),
        ("Artist Wallet", certificate.owner_address),
        ("Artist DID", artwork.artist_did),
        ("Content Hash (SHA-256)", certificate.content_hash_hex),
        ("Medium", artwork.medium),
        ("Year Created", str(artwork.year_created) if artwork.year_created else "-"),
        ("Royalty", f"{certificate.royalty_percentage_bps / 100:.2f}%"),
        ("Royalty Recipient", certificate.royalty_recipient),
        ("Contract Address", certificate.contract_address),
        ("Image (IPFS)", artwork.image_ipfs_uri),
        ("Metadata (IPFS)", artwork.metadata_ipfs_uri),
        ("Transaction", certificate.tx_hash),
        ("Minted At", certificate.minted_at),
        ("Etherscan", etherscan_url),
        ("OpenSea", opensea_url),
        ("Rarible", rarible_url),
    ]

    buffer = io.BytesIO()
    page = canvas.Canvas(buffer, pagesize=A4)
    width, _height = A4
    y = A4[1] - 40 * mm

    page.setFont("Helvetica-Bold", 20)
    page.drawString(20 * mm, y, "Certificate of Authenticity")
    y -= 10 * mm

    page.setFont("Helvetica-Oblique", 13)
    page.drawString(20 * mm, y, _dash_if_empty(artwork.title))
    y -= 10 * mm

    page.setFont("Helvetica", 9)
    for label, value in rows:
        page.setFont("Helvetica-Bold", 9)
        page.drawString(20 * mm, y, label)
        page.setFont("Helvetica", 9)
        page.drawString(65 * mm, y, _dash_if_empty(value)[:110])
        y -= 6 * mm

    page.setFont("Helvetica", 7)
    page.drawString(
        20 * mm, y - 4 * mm,
        "This certificate attests that the photographic work identified by the SHA-256 content "
        "hash above has been registered as an ERC-721 token with EIP-2981 royalty information on "
        "the blockchain identified by the contract address above.",
    )

    page.showPage()
    page.save()
    return buffer.getvalue()
