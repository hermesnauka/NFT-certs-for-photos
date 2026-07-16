"""Composition root: builds the process-wide service singletons, wiring the storage-provider
switch (APP_STORAGE_PROVIDER) the same way the other ports do in their DI container / Program.cs.
Tests substitute fakes by reassigning `certificate_service.storage`/`.contract_service` (see
api/tests/support.py) rather than re-importing this module.
"""

from django.conf import settings

from api.certificate_service import CertificateService
from api.services.blockchain import ContractService
from api.services.identity import MockKycVerificationService
from api.services.storage import LocalStubIpfsStorage, PinataIpfsStorage


def _build_storage():
    config = settings.APP_CONFIG
    return LocalStubIpfsStorage() if config.is_mock_storage else PinataIpfsStorage(config)


storage_service = _build_storage()
kyc_service = MockKycVerificationService()
contract_service = ContractService(settings.APP_CONFIG)
certificate_service = CertificateService(storage_service, contract_service, kyc_service)
