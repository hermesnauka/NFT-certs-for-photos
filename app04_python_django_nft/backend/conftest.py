import pytest

from api import registry
from api.services.storage import LocalStubIpfsStorage
from api.tests.support import FakeContractService


@pytest.fixture(autouse=True)
def fake_external_services():
    """Every test runs against the local-disk storage stub and a fake ContractService instead of
    real Pinata/chain calls (NFR-4), regardless of APP_STORAGE_PROVIDER in the test environment.
    """
    original_storage = registry.certificate_service.storage
    original_contract = registry.certificate_service.contract_service
    registry.certificate_service.storage = LocalStubIpfsStorage()
    fake_contract = FakeContractService()
    registry.certificate_service.contract_service = fake_contract
    try:
        yield fake_contract
    finally:
        registry.certificate_service.storage = original_storage
        registry.certificate_service.contract_service = original_contract
