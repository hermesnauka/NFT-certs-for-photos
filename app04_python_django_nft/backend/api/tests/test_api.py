"""Full HTTP pipeline tests against the real URL routing/view/service graph — only storage and the
chain are faked (see conftest.py's fake_external_services fixture) — covering the happy path end to
end plus the documented error-JSON contract for each reachable 4xx.
"""

import io
import uuid

import pytest
from eth_account import Account
from PIL import Image
from rest_framework.test import APIClient

pytestmark = pytest.mark.django_db


def _random_wallet():
    return Account.create().address


def _random_did():
    return f"did:key:z{uuid.uuid4().hex}"


def _jpeg_bytes():
    image = Image.new("RGB", (4, 4), color="blue")
    buf = io.BytesIO()
    image.save(buf, format="JPEG")
    return buf.getvalue()


def _jpeg_upload_file(filename="photo.jpg"):
    from django.core.files.uploadedfile import SimpleUploadedFile

    return SimpleUploadedFile(filename, _jpeg_bytes(), content_type="image/jpeg")


@pytest.fixture
def client():
    return APIClient()


def test_full_happy_path_upload_through_dashboard_and_pdf(client):
    wallet = _random_wallet()
    did = _random_did()

    upload_response = client.post("/api/uploads", {"file": _jpeg_upload_file()}, format="multipart")
    assert upload_response.status_code == 200
    file_id = upload_response.data["fileId"]
    assert upload_response.data["sha256Hash"]

    verify_response = client.post("/api/identity/verify",
                                   {"walletAddress": wallet, "did": did, "email": "artist@example.com"},
                                   format="json")
    assert verify_response.status_code == 200
    assert verify_response.data["verified"] is True

    artwork_response = client.post("/api/artworks", {
        "fileId": file_id, "title": "Lighthouse at Dusk",
        "description": "A long exposure coastal photograph.", "medium": "Archival pigment print",
        "yearCreated": 2024, "royaltyPercentageBps": 750, "artistWalletAddress": wallet, "artistDid": did,
    }, format="json")
    assert artwork_response.status_code == 201
    artwork_id = artwork_response.data["artworkId"]
    assert artwork_response.data["status"] == "PINNED"
    assert artwork_response.data["imageIpfsUri"].startswith("ipfs://")

    mint_response = client.post(f"/api/artworks/{artwork_id}/mint", {"recipientAddress": wallet}, format="json")
    assert mint_response.status_code == 200
    token_id = mint_response.data["tokenId"]
    assert mint_response.data["ownerAddress"] == wallet
    assert mint_response.data["royaltyPercentageBps"] == 750

    get_response = client.get(f"/api/certificates/{token_id}")
    assert get_response.status_code == 200
    assert get_response.data["artworkId"] == artwork_id

    pdf_response = client.get(f"/api/certificates/{token_id}/pdf")
    assert pdf_response.status_code == 200
    assert pdf_response["Content-Type"] == "application/pdf"
    assert len(pdf_response.content) > 0

    dashboard_response = client.get(f"/api/artists/{wallet}/dashboard")
    assert dashboard_response.status_code == 200
    assert dashboard_response.data["totalCertificates"] == 1
    assert dashboard_response.data["certificates"][0]["tokenId"] == token_id


def test_upload_empty_file_returns_400(client):
    from django.core.files.uploadedfile import SimpleUploadedFile

    response = client.post("/api/uploads", {"file": SimpleUploadedFile("photo.jpg", b"", "image/jpeg")},
                            format="multipart")
    assert response.status_code == 400


def test_upload_unsupported_file_type_returns_400(client):
    response = client.post("/api/uploads", {"file": _jpeg_upload_file("notes.txt")}, format="multipart")
    assert response.status_code == 400


def test_create_artwork_unknown_file_id_returns_404(client):
    response = client.post("/api/artworks", {
        "fileId": str(uuid.uuid4()), "title": "T", "royaltyPercentageBps": 100,
        "artistWalletAddress": _random_wallet(),
    }, format="json")
    assert response.status_code == 404


def test_create_artwork_royalty_out_of_range_returns_422(client):
    upload_response = client.post("/api/uploads", {"file": _jpeg_upload_file()}, format="multipart")

    response = client.post("/api/artworks", {
        "fileId": upload_response.data["fileId"], "title": "T", "royaltyPercentageBps": 10001,
        "artistWalletAddress": _random_wallet(),
    }, format="json")
    assert response.status_code == 422


def test_mint_unknown_artwork_id_returns_404(client):
    response = client.post(f"/api/artworks/{uuid.uuid4()}/mint", {"recipientAddress": _random_wallet()},
                            format="json")
    assert response.status_code == 404


def test_mint_unverified_artist_returns_403(client):
    wallet = _random_wallet()
    upload_response = client.post("/api/uploads", {"file": _jpeg_upload_file()}, format="multipart")
    artwork_response = client.post("/api/artworks", {
        "fileId": upload_response.data["fileId"], "title": "T", "royaltyPercentageBps": 100,
        "artistWalletAddress": wallet,
    }, format="json")

    response = client.post(f"/api/artworks/{artwork_response.data['artworkId']}/mint",
                            {"recipientAddress": wallet}, format="json")
    assert response.status_code == 403


def test_get_certificate_unknown_token_id_returns_404(client):
    assert client.get("/api/certificates/999999").status_code == 404


def test_get_certificate_pdf_unknown_token_id_returns_404(client):
    assert client.get("/api/certificates/999999/pdf").status_code == 404


def test_dashboard_unknown_wallet_returns_empty_not_error(client):
    response = client.get(f"/api/artists/{_random_wallet()}/dashboard")
    assert response.status_code == 200
    assert response.data["totalCertificates"] == 0
    assert response.data["certificates"] == []


@pytest.mark.parametrize("lang", ["en", "pl"])
def test_i18n_supported_language_returns_message_map(client, lang):
    response = client.get(f"/api/i18n/{lang}")
    assert response.status_code == 200
    assert len(response.data) > 0


def test_i18n_unsupported_language_returns_404(client):
    assert client.get("/api/i18n/de").status_code == 404


def test_identity_verify_malformed_wallet_address_returns_400(client):
    response = client.post("/api/identity/verify",
                            {"walletAddress": "not-a-wallet", "did": _random_did(), "email": "a@b.com"},
                            format="json")
    assert response.status_code == 400


def test_identity_verify_malformed_did_returns_400(client):
    response = client.post("/api/identity/verify",
                            {"walletAddress": _random_wallet(), "did": "not-a-did", "email": "a@b.com"},
                            format="json")
    assert response.status_code == 400


def test_error_response_matches_documented_json_contract(client):
    response = client.get("/api/certificates/999999")
    body = response.data

    assert "timestamp" in body
    assert body["status"] == 404
    assert body["error"] == "Not Found"
    assert "message" in body
    assert body["path"] == "/api/certificates/999999"
