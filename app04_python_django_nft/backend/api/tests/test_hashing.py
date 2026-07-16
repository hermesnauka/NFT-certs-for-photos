from api.services.hashing import sha256_hex


def test_sha256_hex_of_empty_input_matches_known_vector():
    assert sha256_hex(b"") == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"


def test_sha256_hex_of_known_input_matches_known_vector():
    assert sha256_hex(b"abc") == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"


def test_sha256_hex_stream_overload_matches_bytes_overload():
    import io

    content = b"streaming hash should match bytes hash"
    assert sha256_hex(content) == sha256_hex(io.BytesIO(content))


def test_sha256_hex_is_deterministic_and_lowercase():
    content = bytes([1, 2, 3, 4, 5])
    digest = sha256_hex(content)

    assert len(digest) == 64
    assert digest == digest.lower()
    assert digest == sha256_hex(content)
