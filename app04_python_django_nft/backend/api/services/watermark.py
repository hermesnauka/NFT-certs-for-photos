"""Embeds a JSON payload referencing the content hash and artist DID into the JPEG's EXIF
ImageDescription tag via Pillow + piexif. Metadata-based (not steganographic) by design — see the
ADR in ../../docs/sdlc/10-glossary-and-decisions.md. Unsupported content types and unreadable
images gracefully pass the original bytes through, matching the other ports' behavior.
"""

import io
import json
import logging

import piexif
from PIL import Image

logger = logging.getLogger(__name__)

JPEG_CONTENT_TYPE = "image/jpeg"


def watermark(original_content, content_type, content_hash, artist_did):
    if content_type.lower() != JPEG_CONTENT_TYPE:
        logger.info(
            "Skipping EXIF watermark embedding for unsupported content type '%s': only %s is supported",
            content_type, JPEG_CONTENT_TYPE,
        )
        return original_content

    try:
        payload_json = json.dumps({"contentHash": content_hash, "artistDid": artist_did})

        image = Image.open(io.BytesIO(original_content))
        image.load()

        try:
            exif_dict = piexif.load(original_content)
        except Exception:
            exif_dict = {"0th": {}, "Exif": {}, "GPS": {}, "1st": {}, "thumbnail": None}
        exif_dict["0th"][piexif.ImageIFD.ImageDescription] = payload_json.encode("utf-8")
        exif_bytes = piexif.dump(exif_dict)

        output = io.BytesIO()
        image.convert("RGB").save(output, format="JPEG", exif=exif_bytes)
        return output.getvalue()
    except Exception:
        logger.warning("Failed to embed EXIF watermark, returning original file content unchanged",
                        exc_info=True)
        return original_content
