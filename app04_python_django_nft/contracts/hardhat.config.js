require("@nomicfoundation/hardhat-toolbox");

/**
 * Hardhat configuration for the PhotoCertificate NFT project.
 *
 * - The default network (used by `npx hardhat test` and `npx hardhat compile`) is Hardhat's
 *   built-in in-memory chain — no separate node process required.
 * - The `localhost` network targets a `npx hardhat node` instance running separately on
 *   127.0.0.1:8545, which `scripts/deploy.js` is meant to be run against via
 *   `npm run deploy:localhost`.
 *
 * See docs/sdlc/06-smart-contract-design.md and docs/sdlc/09-deployment-and-devops.md for the
 * rationale: this phase targets a local Hardhat network only, no public testnet/mainnet.
 */
module.exports = {
  solidity: {
    version: "0.8.24",
    settings: {
      optimizer: {
        enabled: true,
        runs: 200,
      },
      // OpenZeppelin v5's utils/Bytes.sol uses the `mcopy` opcode, which requires
      // targeting the Cancun EVM version (Solidity >=0.8.24 supports emitting it).
      evmVersion: "cancun",
    },
  },
  networks: {
    hardhat: {
      // Default in-memory network used by `npx hardhat test`.
    },
    localhost: {
      url: "http://127.0.0.1:8545",
    },
  },
  paths: {
    sources: "./contracts",
    tests: "./test",
    cache: "./cache",
    artifacts: "./artifacts",
  },
};
