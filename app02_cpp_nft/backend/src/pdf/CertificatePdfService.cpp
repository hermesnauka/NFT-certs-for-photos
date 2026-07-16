#include "pdf/CertificatePdfService.h"

#include <hpdf.h>

#include <csetjmp>
#include <stdexcept>
#include <vector>

namespace nftcerts::pdf {

namespace {

std::string nullToDash(const std::string& value) { return value.empty() ? "-" : value; }

std::string formatBps(int bps) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.2f%%", bps / 100.0);
    return buf;
}

void HPDF_STDCALL errorHandler(HPDF_STATUS errorNo, HPDF_STATUS detailNo, void* userData) {
    std::longjmp(*static_cast<std::jmp_buf*>(userData), 1);
}

}  // namespace

std::vector<unsigned char> CertificatePdfService::generate(const certificate::Certificate& certificate,
                                                             const std::string& etherscanUrl,
                                                             const std::string& openSeaUrl,
                                                             const std::string& raribleUrl) const {
    std::jmp_buf env;
    HPDF_Doc pdf = HPDF_New(errorHandler, &env);
    if (!pdf) {
        throw std::runtime_error("Failed to create PDF document");
    }

    if (setjmp(env)) {
        HPDF_Free(pdf);
        throw std::runtime_error("Failed to generate certificate PDF");
    }

    HPDF_SetCompressionMode(pdf, HPDF_COMP_ALL);
    HPDF_Page page = HPDF_AddPage(pdf);
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);

    const HPDF_REAL kMarginX = 50;
    HPDF_REAL y = 792 - 50;  // A4 portrait height in points minus top margin

    HPDF_Font titleFont = HPDF_GetFont(pdf, "Helvetica-Bold", nullptr);
    HPDF_Font headingFont = HPDF_GetFont(pdf, "Helvetica-Bold", nullptr);
    HPDF_Font labelFont = HPDF_GetFont(pdf, "Helvetica-Bold", nullptr);
    HPDF_Font valueFont = HPDF_GetFont(pdf, "Helvetica", nullptr);
    HPDF_Font bodyFont = HPDF_GetFont(pdf, "Helvetica-Oblique", nullptr);

    HPDF_Page_BeginText(page);
    HPDF_Page_SetFontAndSize(page, titleFont, 22);
    HPDF_Page_TextOut(page, kMarginX, y, "Certificate of Authenticity");
    HPDF_Page_EndText(page);
    y -= 36;

    HPDF_Page_BeginText(page);
    HPDF_Page_SetFontAndSize(page, headingFont, 13);
    HPDF_Page_TextOut(page, kMarginX, y, nullToDash(certificate.artwork.title).c_str());
    HPDF_Page_EndText(page);
    y -= 30;

    auto addRow = [&](const std::string& label, const std::string& value) {
        HPDF_Page_BeginText(page);
        HPDF_Page_SetFontAndSize(page, labelFont, 10);
        HPDF_Page_TextOut(page, kMarginX, y, label.c_str());
        HPDF_Page_SetFontAndSize(page, valueFont, 10);
        HPDF_Page_TextOut(page, kMarginX + 170, y, nullToDash(value).c_str());
        HPDF_Page_EndText(page);
        y -= 20;
    };

    addRow("Token ID", std::to_string(certificate.tokenId));
    addRow("Artist Wallet", certificate.ownerAddress);
    addRow("Artist DID", certificate.artwork.artistDid);
    addRow("Content Hash (SHA-256)", certificate.contentHashHex);
    addRow("Medium", certificate.artwork.medium);
    addRow("Year Created", certificate.artwork.yearCreated.has_value()
                                ? std::to_string(*certificate.artwork.yearCreated)
                                : "-");
    addRow("Royalty", formatBps(certificate.royaltyPercentageBps));
    addRow("Royalty Recipient", certificate.royaltyRecipient);
    addRow("Contract Address", certificate.contractAddress);
    addRow("Image (IPFS)", certificate.artwork.imageIpfsUri);
    addRow("Metadata (IPFS)", certificate.artwork.metadataIpfsUri);
    addRow("Transaction", etherscanUrl);
    addRow("OpenSea", openSeaUrl);
    addRow("Rarible", raribleUrl);
    addRow("Minted At", certificate.mintedAt);

    y -= 20;

    std::string body =
        "This certificate attests that the photograph identified above has been cryptographically "
        "fingerprinted (SHA-256) and permanently recorded on a public blockchain as a non-fungible "
        "token, together with its metadata pinned to IPFS. The recorded content hash and on-chain "
        "royalty configuration (EIP-2981) allow any marketplace or third party to verify the "
        "artwork's authenticity and the creator's right to royalties on secondary sales.";

    HPDF_Page_BeginText(page);
    HPDF_Page_SetFontAndSize(page, bodyFont, 10);
    HPDF_Page_SetTextLeading(page, 14);
    HPDF_Page_MoveTextPos(page, kMarginX, y);
    HPDF_Page_ShowTextNextLine(page, body.c_str());
    HPDF_Page_EndText(page);

    HPDF_SaveToStream(pdf);
    HPDF_UINT32 size = HPDF_GetStreamSize(pdf);
    std::vector<unsigned char> buffer(size);
    HPDF_UINT32 readSize = size;
    HPDF_ReadFromStream(pdf, buffer.data(), &readSize);
    buffer.resize(readSize);

    HPDF_Free(pdf);
    return buffer;
}

}  // namespace nftcerts::pdf
