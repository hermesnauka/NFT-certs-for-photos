#include "i18n/Messages.h"

namespace nftcerts::i18n {

namespace {

const std::map<std::string, std::string> kEnglish = {
    {"dashboard.title", "My Certificates Dashboard"},
    {"dashboard.totalCertificates", "Total Certificates"},
    {"dashboard.royaltiesEarned", "Royalties Earned"},
    {"mint.dropzone.label", "Drag & drop your photograph here, or click to browse"},
    {"mint.hash.explainer",
     "This SHA-256 hash is your artwork's unique digital fingerprint. It cryptographically proves "
     "the file has not been altered and links it to your certificate forever."},
    {"mint.title.label", "Title"},
    {"mint.description.label", "Description"},
    {"mint.medium.label", "Medium"},
    {"mint.year.label", "Year of Creation"},
    {"mint.royalty.label", "Royalty Percentage"},
    {"mint.submit", "Mint Certificate"},
    {"certificate.viewOnOpenSea", "List on OpenSea"},
    {"certificate.viewOnRarible", "View on Rarible"},
    {"certificate.viewOnEtherscan", "View Transaction on Etherscan"},
    {"certificate.downloadPdf", "Download Certificate (PDF)"},
    {"certificate.authenticity.title", "How this certificate protects you"},
    {"certificate.authenticity.body",
     "Your photograph's content hash and ownership are permanently recorded on a public "
     "blockchain, making forgery and unauthorized resale without royalty payment provable and "
     "preventable."},
    {"identity.verify.title", "Verify Your Identity"},
    {"identity.verify.explainer",
     "We confirm you are the rightful copyright holder of this photograph before any certificate "
     "can be minted in your name."},
};

const std::map<std::string, std::string> kPolish = {
    {"dashboard.title", "Panel moich certyfikatów"},
    {"dashboard.totalCertificates", "Liczba certyfikatów"},
    {"dashboard.royaltiesEarned", "Zarobione tantiemy"},
    {"mint.dropzone.label", "Przeciągnij i upuść swoje zdjęcie tutaj, lub kliknij, aby wybrać plik"},
    {"mint.hash.explainer",
     "Ten skrót SHA-256 to unikalny cyfrowy odcisk palca Twojego dzieła. Kryptograficznie dowodzi, "
     "że plik nie został zmieniony i na zawsze łączy go z Twoim certyfikatem."},
    {"mint.title.label", "Tytuł"},
    {"mint.description.label", "Opis"},
    {"mint.medium.label", "Technika"},
    {"mint.year.label", "Rok powstania"},
    {"mint.royalty.label", "Procent tantiem"},
    {"mint.submit", "Wybij certyfikat"},
    {"certificate.viewOnOpenSea", "Wystaw na OpenSea"},
    {"certificate.viewOnRarible", "Zobacz na Rarible"},
    {"certificate.viewOnEtherscan", "Zobacz transakcję na Etherscan"},
    {"certificate.downloadPdf", "Pobierz certyfikat (PDF)"},
    {"certificate.authenticity.title", "Jak ten certyfikat Cię chroni"},
    {"certificate.authenticity.body",
     "Skrót treści Twojej fotografii oraz jej własność są trwale zapisane w publicznym "
     "blockchainie, co czyni podrobienie lub odsprzedaż bez tantiem możliwym do udowodnienia i "
     "zapobieżenia."},
    {"identity.verify.title", "Zweryfikuj swoją tożsamość"},
    {"identity.verify.explainer",
     "Potwierdzamy, że jesteś prawowitym posiadaczem praw autorskich do tej fotografii, zanim "
     "jakikolwiek certyfikat zostanie wybity na Twoje nazwisko."},
};

}  // namespace

const std::map<std::string, std::string>* messagesForLanguage(const std::string& lang) {
    if (lang == "en") return &kEnglish;
    if (lang == "pl") return &kPolish;
    return nullptr;
}

}  // namespace nftcerts::i18n
