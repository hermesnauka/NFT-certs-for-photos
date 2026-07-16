#include "api/ArtistDashboardController.h"
#include "api/ArtworkController.h"
#include "api/CertificateController.h"
#include "api/CertificateDtoMapper.h"
#include "api/ExceptionMapping.h"
#include "api/I18nController.h"
#include "api/IdentityController.h"
#include "api/UploadController.h"
#include "api/UploadStore.h"
#include "blockchain/ContractService.h"
#include "certificate/ArtworkRepository.h"
#include "certificate/CertificateRepository.h"
#include "certificate/CertificateService.h"
#include "config/Config.h"
#include "db/Database.h"
#include "hashing/Sha256HashingService.h"
#include "identity/ArtistIdentityRepository.h"
#include "identity/MockKycVerificationService.h"
#include "pdf/CertificatePdfService.h"
#include "storage/LocalStubIpfsStorageService.h"
#include "storage/PinataIpfsStorageService.h"
#include "watermark/MetadataWatermarkService.h"

#include <drogon/drogon.h>

#include <cstdlib>
#include <iostream>
#include <memory>

int main() {
    nftcerts::config::AppConfig config = nftcerts::config::AppConfig::loadFromEnv();

    try {
        nftcerts::config::validateStartupConfig(config);
    } catch (const std::exception& e) {
        std::cerr << "Startup configuration error: " << e.what() << "\n";
        return 1;
    }

    std::cout << "Storage provider: " << config.storage.provider << "\n";

    nftcerts::db::Database db(config.dbPath);
    nftcerts::db::initSchema(db);

    nftcerts::hashing::Sha256HashingService hashingService;
    nftcerts::watermark::MetadataWatermarkService watermarkService;
    nftcerts::pdf::CertificatePdfService certificatePdfService;

    nftcerts::certificate::ArtworkRepository artworkRepository(db);
    nftcerts::certificate::CertificateRepository certificateRepository(db);
    nftcerts::identity::ArtistIdentityRepository artistIdentityRepository(db);

    nftcerts::identity::MockKycVerificationService kycVerificationService(artistIdentityRepository);

    std::unique_ptr<nftcerts::storage::IpfsStorageService> storageService;
    if (config.storage.isMock()) {
        storageService = std::make_unique<nftcerts::storage::LocalStubIpfsStorageService>();
    } else {
        storageService = std::make_unique<nftcerts::storage::PinataIpfsStorageService>(config.pinata);
    }

    nftcerts::blockchain::ContractService contractService(config.web3);

    nftcerts::certificate::CertificateService certificateService(artworkRepository, certificateRepository,
                                                                   *storageService, contractService,
                                                                   kycVerificationService);

    nftcerts::api::UploadStore uploadStore;
    nftcerts::api::CertificateDtoMapper certificateDtoMapper(config.explorer);

    nftcerts::api::registerExceptionHandler();
    nftcerts::api::registerUploadRoutes(hashingService, watermarkService, uploadStore);
    nftcerts::api::registerArtworkRoutes(uploadStore, certificateService, certificateDtoMapper);
    nftcerts::api::registerCertificateRoutes(certificateService, certificateDtoMapper, certificatePdfService);
    nftcerts::api::registerArtistDashboardRoutes(certificateService, certificateDtoMapper);
    nftcerts::api::registerI18nRoutes();
    nftcerts::api::registerIdentityRoutes(kycVerificationService);

    drogon::app().addListener("0.0.0.0", config.serverPort);
    drogon::app().setClientMaxBodySize(25 * 1024 * 1024);  // 25MB, matches app01's application.yml
    drogon::app().setLogLevel(trantor::Logger::kInfo);

    std::cout << "nft-certs-backend (C++) listening on port " << config.serverPort << "\n";
    drogon::app().run();

    return 0;
}
