import io
import json

import piexif
from PIL import Image

from api.services.watermark import watermark


def _generate_jpeg():
    image = Image.new("RGB", (4, 4), color="red")
    buf = io.BytesIO()
    image.save(buf, format="JPEG")
    return buf.getvalue()


def test_watermark_jpeg_input_embeds_hash_and_did_roundtrippably():
    original = _generate_jpeg()
    content_hash = "9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a0"
    did = "did:key:z6MkhaXgBZDvotDkL5257faiztiGiC2QtKLGpbnnEGta2doK"

    watermarked = watermark(original, "image/jpeg", content_hash, did)

    exif_dict = piexif.load(watermarked)
    description = exif_dict["0th"][piexif.ImageIFD.ImageDescription].decode("utf-8")
    payload = json.loads(description)
    assert payload["contentHash"] == content_hash
    assert payload["artistDid"] == did


def test_watermark_unsupported_content_type_passes_through_unchanged():
    original = b"not actually a png, but content type says so"

    result = watermark(original, "image/png", "somehash", "did:key:zabc")

    assert result == original


def test_watermark_corrupt_jpeg_input_falls_back_to_original_bytes():
    corrupt = bytes([0xFF, 0xD8, 0xFF, 0x00, 0x01, 0x02, 0x03])

    result = watermark(corrupt, "image/jpeg", "somehash", "did:key:zabc")

    assert result == corrupt
