namespace NftCerts.Config;

// Mirrors app01's *Properties classes / app02's AppConfig: environment-variable bound settings
// with a .env fallback (real environment variables always win over the .env file).
public class PinataProperties
{
    public string Jwt { get; set; } = "";
    public string ApiKey { get; set; } = "";
    public string ApiSecret { get; set; } = "";
    public string BaseUrl { get; set; } = "https://api.pinata.cloud";

    public bool HasJwt => Jwt.Length > 0;
    public bool HasApiKeyPair => ApiKey.Length > 0 && ApiSecret.Length > 0;
}

public class Web3Properties
{
    public string RpcUrl { get; set; } = "http://localhost:8545";
    public string MinterPrivateKey { get; set; } = "";
    public string ContractAddress { get; set; } = "";
}

public class ExplorerLinkProperties
{
    public string EtherscanBaseUrl { get; set; } = "http://localhost:8545/tx/";
    public string OpenseaBaseUrl { get; set; } = "https://testnets.opensea.io/assets/";
    public string RaribleBaseUrl { get; set; } = "https://rarible.com/token/";
}

public class StorageProperties
{
    public string Provider { get; set; } = "pinata";

    public bool IsMock => Provider == "mock";
    public bool IsPinata => Provider == "pinata";
}

public class AppConfig
{
    public int ServerPort { get; set; } = 8081;
    public string DbPath { get; set; } = "data/nft-certs.db";
    public PinataProperties Pinata { get; } = new();
    public Web3Properties Web3 { get; } = new();
    public ExplorerLinkProperties Explorer { get; } = new();
    public StorageProperties Storage { get; } = new();

    public static AppConfig LoadFromEnv()
    {
        LoadDotEnv(".env");
        LoadDotEnv("../.env");

        var config = new AppConfig
        {
            ServerPort = int.Parse(EnvOr("SERVER_PORT", "8081")),
            DbPath = EnvOr("DB_PATH", "data/nft-certs.db"),
        };
        config.Storage.Provider = EnvOr("APP_STORAGE_PROVIDER", "pinata");
        config.Pinata.Jwt = EnvOr("PINATA_JWT", "");
        config.Pinata.ApiKey = EnvOr("PINATA_API_KEY", "");
        config.Pinata.ApiSecret = EnvOr("PINATA_API_SECRET", "");
        config.Pinata.BaseUrl = EnvOr("PINATA_BASE_URL", "https://api.pinata.cloud");
        config.Web3.RpcUrl = EnvOr("WEB3J_RPC_URL", "http://localhost:8545");
        config.Web3.MinterPrivateKey = EnvOr("MINTER_PRIVATE_KEY", "");
        config.Web3.ContractAddress = EnvOr("NFT_CONTRACT_ADDRESS", "");
        config.Explorer.EtherscanBaseUrl = EnvOr("ETHERSCAN_BASE_URL", "http://localhost:8545/tx/");
        config.Explorer.OpenseaBaseUrl = EnvOr("OPENSEA_BASE_URL", "https://testnets.opensea.io/assets/");
        config.Explorer.RaribleBaseUrl = EnvOr("RARIBLE_BASE_URL", "https://rarible.com/token/");
        return config;
    }

    // Throws with a descriptive message if required configuration is missing, mirroring app01's
    // StartupConfigurationValidator.
    public static void ValidateStartupConfig(AppConfig config)
    {
        if (!config.Storage.IsMock && !config.Storage.IsPinata)
        {
            throw new InvalidOperationException(
                $"Invalid APP_STORAGE_PROVIDER '{config.Storage.Provider}': must be 'pinata' or 'mock'");
        }
        if (config.Storage.IsPinata && !config.Pinata.HasJwt && !config.Pinata.HasApiKeyPair)
        {
            throw new InvalidOperationException(
                "Missing Pinata credentials: set PINATA_JWT, or both PINATA_API_KEY and PINATA_API_SECRET " +
                "(or set APP_STORAGE_PROVIDER=mock to run without Pinata)");
        }
        if (config.Web3.ContractAddress.Length == 0)
        {
            throw new InvalidOperationException("Missing required environment variable: NFT_CONTRACT_ADDRESS");
        }
        if (config.Web3.MinterPrivateKey.Length == 0)
        {
            throw new InvalidOperationException("Missing required environment variable: MINTER_PRIVATE_KEY");
        }
    }

    private static string EnvOr(string name, string fallback) =>
        Environment.GetEnvironmentVariable(name) is { Length: > 0 } value ? value : fallback;

    // Minimal .env loader: KEY=VALUE lines, '#' comments. Only sets a variable when it is not
    // already present so real environment variables always win (same rule as app01/app02).
    private static void LoadDotEnv(string path)
    {
        if (!File.Exists(path)) return;
        foreach (var rawLine in File.ReadAllLines(path))
        {
            var line = rawLine;
            int hash = line.IndexOf('#');
            if (hash >= 0) line = line[..hash];
            int eq = line.IndexOf('=');
            if (eq < 0) continue;
            string key = line[..eq].Trim();
            string value = line[(eq + 1)..].Trim();
            if (key.Length == 0) continue;
            if (Environment.GetEnvironmentVariable(key) is null)
            {
                Environment.SetEnvironmentVariable(key, value);
            }
        }
    }
}
