namespace NftCerts.I18n;

// Flattened English/Polish message bundles for GET /api/i18n/{lang}, generated verbatim from
// app01's messages_en.properties / messages_pl.properties. Regenerate from those files rather
// than editing by hand.
public static class Messages
{
    // Returns the message map for "en" or "pl", or null if unsupported.
    public static IReadOnlyDictionary<string, string>? ForLanguage(string lang) => lang switch
    {
        "en" => English,
        "pl" => Polish,
        _ => null,
    };

    private static readonly Dictionary<string, string> English = new()
    {
        ["certificate.authenticity.body"] = "Your photograph's content hash and ownership are permanently recorded on a public blockchain, making forgery and unauthorized resale without royalty payment provable and preventable.",
        ["certificate.authenticity.title"] = "How this certificate protects you",
        ["certificate.downloadPdf"] = "Download Certificate (PDF)",
        ["certificate.viewOnEtherscan"] = "View Transaction on Etherscan",
        ["certificate.viewOnOpenSea"] = "List on OpenSea",
        ["certificate.viewOnRarible"] = "View on Rarible",
        ["dashboard.royaltiesEarned"] = "Royalties Earned",
        ["dashboard.title"] = "My Certificates Dashboard",
        ["dashboard.totalCertificates"] = "Total Certificates",
        ["identity.verify.explainer"] = "We confirm you are the rightful copyright holder of this photograph before any certificate can be minted in your name.",
        ["identity.verify.title"] = "Verify Your Identity",
        ["mint.description.label"] = "Description",
        ["mint.dropzone.label"] = "Drag & drop your photograph here, or click to browse",
        ["mint.hash.explainer"] = "This SHA-256 hash is your artwork's unique digital fingerprint. It cryptographically proves the file has not been altered and links it to your certificate forever.",
        ["mint.medium.label"] = "Medium",
        ["mint.royalty.label"] = "Royalty Percentage",
        ["mint.submit"] = "Mint Certificate",
        ["mint.title.label"] = "Title",
        ["mint.year.label"] = "Year of Creation",
    };

    private static readonly Dictionary<string, string> Polish = new()
    {
        ["certificate.authenticity.body"] = "Skrót treści Twojej fotografii oraz jej własność są trwale zapisane w publicznym blockchainie, co czyni podrobienie lub odsprzedaż bez tantiem możliwym do udowodnienia i zapobieżenia.",
        ["certificate.authenticity.title"] = "Jak ten certyfikat Cię chroni",
        ["certificate.downloadPdf"] = "Pobierz certyfikat (PDF)",
        ["certificate.viewOnEtherscan"] = "Zobacz transakcję na Etherscan",
        ["certificate.viewOnOpenSea"] = "Wystaw na OpenSea",
        ["certificate.viewOnRarible"] = "Zobacz na Rarible",
        ["dashboard.royaltiesEarned"] = "Zarobione tantiemy",
        ["dashboard.title"] = "Panel moich certyfikatów",
        ["dashboard.totalCertificates"] = "Liczba certyfikatów",
        ["identity.verify.explainer"] = "Potwierdzamy, że jesteś prawowitym posiadaczem praw autorskich do tej fotografii, zanim jakikolwiek certyfikat zostanie wybity na Twoje nazwisko.",
        ["identity.verify.title"] = "Zweryfikuj swoją tożsamość",
        ["mint.description.label"] = "Opis",
        ["mint.dropzone.label"] = "Przeciągnij i upuść swoje zdjęcie tutaj, lub kliknij, aby wybrać plik",
        ["mint.hash.explainer"] = "Ten skrót SHA-256 to unikalny cyfrowy odcisk palca Twojego dzieła. Kryptograficznie dowodzi, że plik nie został zmieniony i na zawsze łączy go z Twoim certyfikatem.",
        ["mint.medium.label"] = "Technika",
        ["mint.royalty.label"] = "Procent tantiem",
        ["mint.submit"] = "Wybij certyfikat",
        ["mint.title.label"] = "Tytuł",
        ["mint.year.label"] = "Rok powstania",
    };
}
