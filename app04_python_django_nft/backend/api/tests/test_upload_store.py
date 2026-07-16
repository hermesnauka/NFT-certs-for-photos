import pytest

from api.errors import FileNotFoundInUploadStoreException
from api.upload_store import StoredUpload, UploadStore


def test_store_then_get_returns_the_same_upload():
    store = UploadStore()
    upload = StoredUpload("photo.jpg", "image/jpeg", "abc123", b"bytes")

    file_id = store.store(upload)

    assert store.get(file_id) is upload


def test_get_unknown_file_id_raises():
    store = UploadStore()

    with pytest.raises(FileNotFoundInUploadStoreException):
        store.get("does-not-exist")
