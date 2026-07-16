"""Maps a Certificate model to the API DTO shared with app01/app02/app03, including marketplace
deep links. See ../docs/sdlc/05-api-design.md §3-4.
"""

from django.conf import settings


def build_etherscan_url(certificate):
    return settings.APP_CONFIG.etherscan_base_url + certificate.tx_hash


def build_opensea_url(certificate):
    return f"{settings.APP_CONFIG.opensea_base_url}{certificate.contract_address}/{certificate.token_id}"


def build_rarible_url(certificate):
    return f"{settings.APP_CONFIG.rarible_base_url}{certificate.contract_address}:{certificate.token_id}"


def to_dto(certificate):
    artwork = certificate.artwork
    return {
        "tokenId": certificate.token_id,
        "artworkId": str(artwork.id),
        "title": artwork.title,
        "contentHashHex": certificate.content_hash_hex,
        "contractAddress": certificate.contract_address,
        "txHash": certificate.tx_hash,
        "ownerAddress": certificate.owner_address,
        "royaltyPercentageBps": certificate.royalty_percentage_bps,
        "imageIpfsUri": artwork.image_ipfs_uri,
        "metadataIpfsUri": artwork.metadata_ipfs_uri,
        "etherscanUrl": build_etherscan_url(certificate),
        "openSeaUrl": build_opensea_url(certificate),
        "raribleUrl": build_rarible_url(certificate),
    }
