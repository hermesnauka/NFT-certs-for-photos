import json

import pytest

from api.errors import IpfsPinningException
from api.services.hashing import sha256_hex
from api.services.storage import LocalStubIpfsStorage, PinataIpfsStorage


class FakeConfig:
    def __init__(self, jwt="", api_key="", api_secret="", base_url="https://api.pinata.cloud"):
        self.pinata_jwt = jwt
        self.pinata_api_key = api_key
        self.pinata_api_secret = api_secret
        self.pinata_base_url = base_url


class FakeResponse:
    def __init__(self, status_code, json_body=None, text=""):
        self.status_code = status_code
        self.ok = 200 <= status_code < 300
        self._json_body = json_body
        self.text = text or json.dumps(json_body or {})

    def json(self):
        if self._json_body is None:
            raise ValueError("no json body")
        return self._json_body


class FakeSession:
    def __init__(self, respond):
        self._respond = respond
        self.last_request = None

    def post(self, url, headers=None, timeout=None, **kwargs):
        self.last_request = {"url": url, "headers": headers, **kwargs}
        return self._respond(self.last_request)


class TestLocalStubIpfsStorage:
    def test_pin_file_returns_deterministic_stub_uri(self):
        storage = LocalStubIpfsStorage()
        content = b"hello world"

        result = storage.pin_file(content, "hello.txt", "text/plain")

        expected_cid = "stub-" + sha256_hex(content)
        assert result.cid == expected_cid
        assert result.uri == f"ipfs://{expected_cid}"

    def test_pin_json_returns_deterministic_stub_uri(self):
        storage = LocalStubIpfsStorage()
        payload = {"title": "Lighthouse at Dusk"}

        result = storage.pin_json(payload, "metadata")

        assert result.cid == "stub-" + sha256_hex(json.dumps(payload).encode("utf-8"))

    def test_pin_file_same_content_twice_returns_same_cid(self):
        storage = LocalStubIpfsStorage()
        content = b"same bytes"

        first = storage.pin_file(content, "a.txt", "text/plain")
        second = storage.pin_file(content, "b.txt", "text/plain")

        assert first.cid == second.cid


class TestPinataIpfsStorage:
    def test_pin_file_with_jwt_sends_bearer_auth_and_returns_cid(self):
        session = FakeSession(lambda req: FakeResponse(200, {"IpfsHash": "bafyImageCid"}))
        storage = PinataIpfsStorage(FakeConfig(jwt="test-jwt"), session=session)

        result = storage.pin_file(b"123", "photo.jpg", "image/jpeg")

        assert result.cid == "bafyImageCid"
        assert result.uri == "ipfs://bafyImageCid"
        assert session.last_request["headers"]["Authorization"] == "Bearer test-jwt"

    def test_pin_json_with_api_key_pair_sends_key_secret_headers(self):
        session = FakeSession(lambda req: FakeResponse(200, {"IpfsHash": "bafyMetadataCid"}))
        storage = PinataIpfsStorage(FakeConfig(api_key="key123", api_secret="secret456"), session=session)

        result = storage.pin_json({"a": 1}, "metadata")

        assert result.cid == "bafyMetadataCid"
        assert session.last_request["headers"]["pinata_api_key"] == "key123"
        assert session.last_request["headers"]["pinata_secret_api_key"] == "secret456"

    def test_pin_file_no_credentials_configured_raises_without_sending_request(self):
        session = FakeSession(lambda req: pytest.fail("should not be called"))
        storage = PinataIpfsStorage(FakeConfig(), session=session)

        with pytest.raises(IpfsPinningException):
            storage.pin_file(b"1", "a.jpg", "image/jpeg")

    def test_pin_file_non_success_status_code_raises(self):
        session = FakeSession(lambda req: FakeResponse(500, text="oops"))
        storage = PinataIpfsStorage(FakeConfig(jwt="jwt"), session=session)

        with pytest.raises(IpfsPinningException):
            storage.pin_file(b"1", "a.jpg", "image/jpeg")

    def test_pin_file_response_missing_ipfs_hash_raises(self):
        session = FakeSession(lambda req: FakeResponse(200, {"unexpected": "shape"}))
        storage = PinataIpfsStorage(FakeConfig(jwt="jwt"), session=session)

        with pytest.raises(IpfsPinningException):
            storage.pin_file(b"1", "a.jpg", "image/jpeg")
