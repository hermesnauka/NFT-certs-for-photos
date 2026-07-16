import re

WALLET_ADDRESS_PATTERN = re.compile(r"^0x[a-fA-F0-9]{40}$")
DID_PATTERN = re.compile(r"^did:[a-zA-Z0-9]+:.+$")
