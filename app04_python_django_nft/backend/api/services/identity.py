"""Swap-in point for a real DID/KYC provider (Persona, Civic, ...). Mirrors the other ports'
KycVerificationService. The mock implementation always approves verification.
"""

import datetime
from abc import ABC, abstractmethod

from api.models import ArtistIdentity


class KycVerificationService(ABC):
    @abstractmethod
    def verify(self, wallet_address, did, email):
        ...

    @abstractmethod
    def is_verified(self, wallet_address):
        ...


class MockKycVerificationService(KycVerificationService):
    def verify(self, wallet_address, did, email):
        identity, _created = ArtistIdentity.objects.get_or_create(wallet_address=wallet_address)
        identity.did = did
        identity.email = email
        identity.verified = True
        identity.verified_at = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
        identity.save()
        return identity

    def is_verified(self, wallet_address):
        identity = ArtistIdentity.objects.filter(wallet_address=wallet_address).first()
        return identity is not None and identity.verified
