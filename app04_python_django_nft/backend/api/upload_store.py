"""In-memory store for uploaded (hashed/watermarked) file bytes, retrievable later by fileId when
creating an artwork. Mirrors the other ports' UploadStore.
"""

import threading
import uuid
from dataclasses import dataclass

from api.errors import FileNotFoundInUploadStoreException


@dataclass
class StoredUpload:
    original_filename: str
    content_type: str
    sha256_hash: str
    content: bytes


class UploadStore:
    def __init__(self):
        self._lock = threading.Lock()
        self._uploads = {}

    def store(self, upload):
        file_id = str(uuid.uuid4())
        with self._lock:
            self._uploads[file_id] = upload
        return file_id

    def get(self, file_id):
        with self._lock:
            upload = self._uploads.get(file_id)
        if upload is None:
            raise FileNotFoundInUploadStoreException(file_id)
        return upload


# Process-wide singleton, mirrors UploadStore being registered as a DI singleton in the other ports.
upload_store = UploadStore()
