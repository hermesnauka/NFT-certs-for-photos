"""Streaming SHA-256 hashing of uploaded files (NFR-1)."""

import hashlib


def sha256_hex(content):
    """content: bytes or a binary file-like object opened for reading."""
    digest = hashlib.sha256()
    if hasattr(content, "read"):
        for chunk in iter(lambda: content.read(65536), b""):
            digest.update(chunk)
    else:
        digest.update(content)
    return digest.hexdigest()
