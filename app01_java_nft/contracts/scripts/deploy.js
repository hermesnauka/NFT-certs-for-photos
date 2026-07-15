const hre = require("hardhat");
const fs = require("fs");
const path = require("path");

const CONTRACT_NAME = "Fine Art Photo Certificate";
const CONTRACT_SYMBOL = "PHOTOCERT";

/**
 * Deploys PhotoCertificate to whichever network Hardhat is currently pointed at (e.g.
 * `--network localhost` against a separately-running `npx hardhat node`).
 *
 * Optionally grants MINTER_ROLE to a backend-controlled account, read from the
 * BACKEND_MINTER_ADDRESS environment variable, so a locally-run Java backend (ContractService)
 * can call mintCertificate without holding the deployer's key.
 *
 * NOTE: The deployment info written to deployments/localhost.json below is for human/manual
 * reference only (e.g. copy-pasting the address into the backend's NFT_CONTRACT_ADDRESS config).
 * The Java backend does NOT read this file automatically in this phase — it has its own
 * manually-encoded ABI calls (see docs/sdlc/06-smart-contract-design.md /
 * docs/sdlc/09-deployment-and-devops.md).
 */
async function main() {
  const [deployer] = await hre.ethers.getSigners();
  console.log(`Deploying PhotoCertificate with deployer account: ${deployer.address}`);

  const PhotoCertificate = await hre.ethers.getContractFactory("PhotoCertificate");
  const contract = await PhotoCertificate.deploy(CONTRACT_NAME, CONTRACT_SYMBOL);
  await contract.waitForDeployment();

  const address = await contract.getAddress();
  console.log("=================================================================");
  console.log(`PhotoCertificate deployed to: ${address}`);
  console.log("=================================================================");

  const backendMinterAddress = process.env.BACKEND_MINTER_ADDRESS;
  if (backendMinterAddress) {
    const MINTER_ROLE = await contract.MINTER_ROLE();
    const tx = await contract.grantRole(MINTER_ROLE, backendMinterAddress);
    await tx.wait();
    console.log(`Granted MINTER_ROLE to BACKEND_MINTER_ADDRESS: ${backendMinterAddress}`);
  } else {
    console.warn(
      "WARNING: BACKEND_MINTER_ADDRESS is not set. Only the deployer account " +
        `(${deployer.address}) holds MINTER_ROLE. Set BACKEND_MINTER_ADDRESS and re-run, or ` +
        "grant the role manually, before pointing the backend's ContractService at this contract."
    );
  }

  // Write deployment info for human/manual reference (deployments/ dir).
  const deploymentsDir = path.join(__dirname, "..", "deployments");
  if (!fs.existsSync(deploymentsDir)) {
    fs.mkdirSync(deploymentsDir, { recursive: true });
  }

  const artifact = await hre.artifacts.readArtifact("PhotoCertificate");
  const deploymentInfo = {
    address,
    abi: artifact.abi,
  };

  const outputPath = path.join(deploymentsDir, "localhost.json");
  fs.writeFileSync(outputPath, JSON.stringify(deploymentInfo, null, 2));
  console.log(`Deployment info written to: ${outputPath}`);
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
