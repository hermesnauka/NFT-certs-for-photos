"""Abstraction over decentralized (IPFS) storage pinning. Two implementations: PinataIpfsStorage
(real, used outside the mock storage profile) and LocalStubIpfsStorage (local stub, wired only
when APP_STORAGE_PROVIDER=mock). Mirrors the other ports' IpfsStorageService.
"""

import json
import os
import tempfile
from abc import ABC, abstractmethod
from dataclasses import dataclass

import requests

from api.errors import IpfsPinningException
from api.services.hashing import sha256_hex


@dataclass
class PinResult:
    cid: str
    uri: str

    @staticmethod
    def of(cid):
        return PinResult(cid, f"ipfs://{cid}")


class IpfsStorageService(ABC):
    @abstractmethod
    def pin_file(self, content, filename, content_type):
        """Pins raw file bytes (e.g. the source image) to IPFS."""

    @abstractmethod
    def pin_json(self, payload, name):
        """Pins a JSON-serializable payload (e.g. NFT metadata) to IPFS."""


class PinataIpfsStorage(IpfsStorageService):
    PIN_FILE_PATH = "/pinning/pinFileToIPFS"
    PIN_JSON_PATH = "/pinning/pinJSONToIPFS"

    def __init__(self, config, session=None):
        self._config = config
        self._session = session or requests

    def pin_file(self, content, filename, content_type):
        files = {"file": (filename, content, content_type or "application/octet-stream")}
        return self._send(self.PIN_FILE_PATH, "file", files=files)

    def pin_json(self, payload, name):
        body = {"pinataMetadata": {"name": name}, "pinataContent": payload}
        return self._send(self.PIN_JSON_PATH, "JSON", json=body)

    def _auth_headers(self):
        if len(self._config.pinata_jwt) > 0:
            return {"Authorization": f"Bearer {self._config.pinata_jwt}"}
        if len(self._config.pinata_api_key) > 0 and len(self._config.pinata_api_secret) > 0:
            return {
                "pinata_api_key": self._config.pinata_api_key,
                "pinata_secret_api_key": self._config.pinata_api_secret,
            }
        raise IpfsPinningException(
            "No Pinata credentials configured (PINATA_JWT or PINATA_API_KEY/PINATA_API_SECRET)"
        )

    def _send(self, path, what, **kwargs):
        headers = self._auth_headers()
        url = self._config.pinata_base_url.rstrip("/") + path
        try:
            response = self._session.post(url, headers=headers, timeout=30, **kwargs)
        except requests.RequestException:
            raise IpfsPinningException(f"Failed to pin {what} to IPFS via Pinata: request failed")

        if not response.ok:
            raise IpfsPinningException(f"Failed to pin {what} to IPFS via Pinata: HTTP {response.status_code}")
        return PinResult.of(self._extract_cid(response))

    @staticmethod
    def _extract_cid(response):
        try:
            body = response.json()
        except ValueError:
            raise IpfsPinningException(f"Failed to parse Pinata response: {response.text}")
        cid = body.get("IpfsHash") if isinstance(body, dict) else None
        if not cid:
            raise IpfsPinningException(f"Pinata response did not contain an IpfsHash: {response.text}")
        return cid


class LocalStubIpfsStorage(IpfsStorageService):
    """Offline stand-in for Pinata: writes the pinned bytes to a local temp directory and returns a
    deterministic fake "ipfs://stub-<sha256>" URI.
    """

    def __init__(self):
        self._storage_dir = os.path.join(tempfile.gettempdir(), "local-stub-ipfs")
        os.makedirs(self._storage_dir, exist_ok=True)

    def pin_file(self, content, filename, content_type):
        return self._store_and_build_result(content)

    def pin_json(self, payload, name):
        return self._store_and_build_result(json.dumps(payload).encode("utf-8"))

    def _store_and_build_result(self, content):
        cid = "stub-" + sha256_hex(content)
        with open(os.path.join(self._storage_dir, cid), "wb") as f:
            f.write(content)
        return PinResult.of(cid)
