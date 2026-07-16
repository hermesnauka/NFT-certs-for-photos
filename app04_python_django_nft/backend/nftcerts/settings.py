"""
Django settings for the nftcerts project (app04_python_django_nft).

See ../docs/sdlc/03-architecture.md and 09-deployment-and-devops.md for the platform-level
config this loads (storage provider switch, Pinata/Web3 credentials, etc).
"""

import os
import sys
from pathlib import Path

from dotenv import load_dotenv

BASE_DIR = Path(__file__).resolve().parent.parent

# Real environment variables always win over the .env file (same rule as the other ports).
load_dotenv(BASE_DIR / ".env", override=False)
load_dotenv(BASE_DIR.parent / ".env", override=False)

SECRET_KEY = "django-insecure-nft-certs-for-photos-course-reference-app04"

DEBUG = True

ALLOWED_HOSTS = ["*"]

INSTALLED_APPS = [
    "django.contrib.contenttypes",
    "django.contrib.auth",  # required by DRF's request.user/AnonymousUser resolution, even unused
    "django.contrib.staticfiles",
    "rest_framework",
    "api",
]

MIDDLEWARE = [
    "django.middleware.common.CommonMiddleware",
]

ROOT_URLCONF = "nftcerts.urls"

TEMPLATES = [
    {
        "BACKEND": "django.template.backends.django.DjangoTemplates",
        "DIRS": [],
        "APP_DIRS": True,
        "OPTIONS": {"context_processors": []},
    },
]

WSGI_APPLICATION = "nftcerts.wsgi.application"

DATABASES = {
    "default": {
        "ENGINE": "django.db.backends.sqlite3",
        "NAME": BASE_DIR / "db.sqlite3",
    }
}

USE_I18N = False  # this project ships its own EN/PL message maps via GET /api/i18n/{lang}
USE_TZ = True
TIME_ZONE = "UTC"

STATIC_URL = "static/"

DEFAULT_AUTO_FIELD = "django.db.models.BigAutoField"

REST_FRAMEWORK = {
    "EXCEPTION_HANDLER": "api.errors.exception_handler",
    "DEFAULT_RENDERER_CLASSES": ["rest_framework.renderers.JSONRenderer"],
    # No login for this API (the "user" is a wallet address, verified via POST /api/identity/verify,
    # not a Django auth user) — keeps django.contrib.auth out of INSTALLED_APPS entirely.
    "DEFAULT_AUTHENTICATION_CLASSES": [],
    "DEFAULT_PERMISSION_CLASSES": ["rest_framework.permissions.AllowAny"],
}

# Running under pytest/manage.py test: skip fail-fast Pinata/Web3 validation (NFR-3 applies to the
# real run path only) and force the local storage/chain fakes regardless of ambient env vars.
TESTING = "pytest" in sys.modules or "test" in sys.argv


def _env(name, default=""):
    return os.environ.get(name, default)


class PlatformConfig:
    """Environment-derived platform config; mirrors AppConfig in the other three ports."""

    def __init__(self):
        self.server_port = int(_env("SERVER_PORT", "8000"))
        self.db_path = str(BASE_DIR / "db.sqlite3")
        self.storage_provider = _env("APP_STORAGE_PROVIDER", "pinata")
        self.pinata_jwt = _env("PINATA_JWT")
        self.pinata_api_key = _env("PINATA_API_KEY")
        self.pinata_api_secret = _env("PINATA_API_SECRET")
        self.pinata_base_url = _env("PINATA_BASE_URL", "https://api.pinata.cloud")
        self.web3_rpc_url = _env("WEB3J_RPC_URL", "http://localhost:8545")
        self.minter_private_key = _env("MINTER_PRIVATE_KEY")
        self.nft_contract_address = _env("NFT_CONTRACT_ADDRESS")
        self.etherscan_base_url = _env("ETHERSCAN_BASE_URL", "http://localhost:8545/tx/")
        self.opensea_base_url = _env("OPENSEA_BASE_URL", "https://testnets.opensea.io/assets/")
        self.rarible_base_url = _env("RARIBLE_BASE_URL", "https://rarible.com/token/")

    @property
    def is_mock_storage(self):
        return self.storage_provider == "mock"

    @property
    def has_pinata_jwt(self):
        return len(self.pinata_jwt) > 0

    @property
    def has_pinata_key_pair(self):
        return len(self.pinata_api_key) > 0 and len(self.pinata_api_secret) > 0

    def validate(self):
        if self.storage_provider not in ("pinata", "mock"):
            raise RuntimeError(
                f"Invalid APP_STORAGE_PROVIDER '{self.storage_provider}': must be 'pinata' or 'mock'"
            )
        if self.storage_provider == "pinata" and not self.has_pinata_jwt and not self.has_pinata_key_pair:
            raise RuntimeError(
                "Missing Pinata credentials: set PINATA_JWT, or both PINATA_API_KEY and "
                "PINATA_API_SECRET (or set APP_STORAGE_PROVIDER=mock to run without Pinata)"
            )
        if not self.nft_contract_address:
            raise RuntimeError("Missing required environment variable: NFT_CONTRACT_ADDRESS")
        if not self.minter_private_key:
            raise RuntimeError("Missing required environment variable: MINTER_PRIVATE_KEY")


APP_CONFIG = PlatformConfig()

if not TESTING and not APP_CONFIG.is_mock_storage:
    APP_CONFIG.validate()

print(f"Active storage provider: {APP_CONFIG.storage_provider}")
