const { expect } = require("chai");
const { ethers } = require("hardhat");
const {
  loadFixture,
} = require("@nomicfoundation/hardhat-network-helpers");

// Well-known ERC-165 interface IDs (per the respective EIPs).
const INTERFACE_ID_ERC721 = "0x80ac58cd";
const INTERFACE_ID_ERC2981 = "0x2a55205a";
const INTERFACE_ID_ACCESS_CONTROL = "0x7965db0b";

const NAME = "Fine Art Photo Certificate";
const SYMBOL = "PHOTOCERT";

function hashOf(label) {
  return ethers.keccak256(ethers.toUtf8Bytes(label));
}

describe("PhotoCertificate", function () {
  async function deployFixture() {
    const [deployer, minter, pauser, artist, otherArtist, royaltyRecipient, stranger] =
      await ethers.getSigners();

    const PhotoCertificate = await ethers.getContractFactory("PhotoCertificate");
    const contract = await PhotoCertificate.deploy(NAME, SYMBOL);
    await contract.waitForDeployment();

    const MINTER_ROLE = await contract.MINTER_ROLE();
    const PAUSER_ROLE = await contract.PAUSER_ROLE();
    const DEFAULT_ADMIN_ROLE = await contract.DEFAULT_ADMIN_ROLE();

    return {
      contract,
      deployer,
      minter,
      pauser,
      artist,
      otherArtist,
      royaltyRecipient,
      stranger,
      MINTER_ROLE,
      PAUSER_ROLE,
      DEFAULT_ADMIN_ROLE,
    };
  }

  describe("Deployment & roles", function () {
    it("grants the deployer DEFAULT_ADMIN_ROLE, MINTER_ROLE, and PAUSER_ROLE", async function () {
      const { contract, deployer, MINTER_ROLE, PAUSER_ROLE, DEFAULT_ADMIN_ROLE } =
        await loadFixture(deployFixture);

      expect(await contract.hasRole(DEFAULT_ADMIN_ROLE, deployer.address)).to.be.true;
      expect(await contract.hasRole(MINTER_ROLE, deployer.address)).to.be.true;
      expect(await contract.hasRole(PAUSER_ROLE, deployer.address)).to.be.true;
    });

    it("sets name and symbol", async function () {
      const { contract } = await loadFixture(deployFixture);
      expect(await contract.name()).to.equal(NAME);
      expect(await contract.symbol()).to.equal(SYMBOL);
    });

    it("allows the admin to grant/revoke MINTER_ROLE and PAUSER_ROLE, verified via hasRole", async function () {
      const { contract, deployer, minter, MINTER_ROLE, PAUSER_ROLE } = await loadFixture(
        deployFixture
      );

      expect(await contract.hasRole(MINTER_ROLE, minter.address)).to.be.false;

      await contract.connect(deployer).grantRole(MINTER_ROLE, minter.address);
      expect(await contract.hasRole(MINTER_ROLE, minter.address)).to.be.true;

      await contract.connect(deployer).revokeRole(MINTER_ROLE, minter.address);
      expect(await contract.hasRole(MINTER_ROLE, minter.address)).to.be.false;

      await contract.connect(deployer).grantRole(PAUSER_ROLE, minter.address);
      expect(await contract.hasRole(PAUSER_ROLE, minter.address)).to.be.true;
    });

    it("reverts if a non-admin tries to grant a role", async function () {
      const { contract, stranger, artist, MINTER_ROLE } = await loadFixture(deployFixture);
      await expect(
        contract.connect(stranger).grantRole(MINTER_ROLE, artist.address)
      ).to.be.reverted;
    });
  });

  describe("mintCertificate", function () {
    it("succeeds when called by an address with MINTER_ROLE: emits event, sets owner, hash, and URI", async function () {
      const { contract, deployer, artist, royaltyRecipient } = await loadFixture(deployFixture);

      const contentHash = hashOf("photo-1");
      const metadataURI = "ipfs://bafy.../metadata.json";
      const royaltyBps = 500; // 5%

      const tx = await contract
        .connect(deployer)
        .mintCertificate(artist.address, metadataURI, contentHash, royaltyRecipient.address, royaltyBps);
      const receipt = await tx.wait();

      // Expect tokenId 1 (first mint, counter starts at 1).
      await expect(tx)
        .to.emit(contract, "CertificateMinted")
        .withArgs(1n, artist.address, contentHash, metadataURI);

      expect(await contract.ownerOf(1)).to.equal(artist.address);
      expect(await contract.tokenContentHash(1)).to.equal(contentHash);
      expect(await contract.tokenURI(1)).to.equal(metadataURI);
      expect(await contract.hashRegistered(contentHash)).to.be.true;
      expect(receipt.status).to.equal(1);
    });

    it("assigns incrementing token IDs across multiple mints", async function () {
      const { contract, deployer, artist, royaltyRecipient } = await loadFixture(deployFixture);

      await contract
        .connect(deployer)
        .mintCertificate(artist.address, "ipfs://a", hashOf("a"), royaltyRecipient.address, 100);
      const tx2 = await contract
        .connect(deployer)
        .mintCertificate(artist.address, "ipfs://b", hashOf("b"), royaltyRecipient.address, 100);

      await expect(tx2).to.emit(contract, "CertificateMinted").withArgs(2n, artist.address, hashOf("b"), "ipfs://b");
    });

    it("rejects a second mint with a duplicate content hash", async function () {
      const { contract, deployer, artist, otherArtist, royaltyRecipient } = await loadFixture(
        deployFixture
      );

      const contentHash = hashOf("duplicate-photo");

      await contract
        .connect(deployer)
        .mintCertificate(artist.address, "ipfs://first", contentHash, royaltyRecipient.address, 250);

      await expect(
        contract
          .connect(deployer)
          .mintCertificate(
            otherArtist.address,
            "ipfs://second",
            contentHash,
            royaltyRecipient.address,
            250
          )
      ).to.be.revertedWith("PhotoCertificate: duplicate content hash");

      // First mint's data remains untouched.
      expect(await contract.ownerOf(1)).to.equal(artist.address);
    });

    it("reverts when called by an address without MINTER_ROLE", async function () {
      const { contract, stranger, artist, royaltyRecipient } = await loadFixture(deployFixture);

      await expect(
        contract
          .connect(stranger)
          .mintCertificate(artist.address, "ipfs://x", hashOf("x"), royaltyRecipient.address, 100)
      ).to.be.reverted;
    });
  });

  describe("Royalty math (EIP-2981)", function () {
    const cases = [
      { bps: 500, salePrice: ethers.parseEther("1") }, // 5%
      { bps: 0, salePrice: ethers.parseEther("1") }, // 0 bps edge case
      { bps: 10000, salePrice: ethers.parseEther("2") }, // 100% edge case
      { bps: 1234, salePrice: 1_000_000n },
    ];

    for (const { bps, salePrice } of cases) {
      it(`computes royaltyInfo correctly for ${bps} bps on sale price ${salePrice.toString()}`, async function () {
        const { contract, deployer, artist, royaltyRecipient } = await loadFixture(deployFixture);

        const contentHash = hashOf(`royalty-${bps}-${salePrice}`);
        await contract
          .connect(deployer)
          .mintCertificate(artist.address, "ipfs://royalty", contentHash, royaltyRecipient.address, bps);

        const [receiver, royaltyAmount] = await contract.royaltyInfo(1, salePrice);

        expect(receiver).to.equal(royaltyRecipient.address);
        expect(royaltyAmount).to.equal((salePrice * BigInt(bps)) / 10000n);
      });
    }
  });

  describe("Access control", function () {
    it("reverts pause() when called without PAUSER_ROLE", async function () {
      const { contract, stranger } = await loadFixture(deployFixture);
      await expect(contract.connect(stranger).pause()).to.be.reverted;
    });

    it("reverts unpause() when called without PAUSER_ROLE", async function () {
      const { contract, deployer, stranger } = await loadFixture(deployFixture);
      await contract.connect(deployer).pause();
      await expect(contract.connect(stranger).unpause()).to.be.reverted;
    });
  });

  describe("Pausing", function () {
    it("blocks mintCertificate while paused, and unpause() restores it", async function () {
      const { contract, deployer, artist, royaltyRecipient } = await loadFixture(deployFixture);

      await contract.connect(deployer).pause();
      expect(await contract.paused()).to.be.true;

      await expect(
        contract
          .connect(deployer)
          .mintCertificate(artist.address, "ipfs://p", hashOf("p"), royaltyRecipient.address, 100)
      ).to.be.revertedWithCustomError(contract, "EnforcedPause");

      await contract.connect(deployer).unpause();
      expect(await contract.paused()).to.be.false;

      await expect(
        contract
          .connect(deployer)
          .mintCertificate(artist.address, "ipfs://p", hashOf("p"), royaltyRecipient.address, 100)
      ).to.not.be.reverted;
    });

    it("blocks transferFrom/safeTransferFrom while paused, and unpause() restores it", async function () {
      const { contract, deployer, artist, otherArtist, royaltyRecipient } = await loadFixture(
        deployFixture
      );

      await contract
        .connect(deployer)
        .mintCertificate(artist.address, "ipfs://t", hashOf("t"), royaltyRecipient.address, 100);

      await contract.connect(deployer).pause();

      await expect(
        contract
          .connect(artist)
          .transferFrom(artist.address, otherArtist.address, 1)
      ).to.be.revertedWith("Pausable: paused");

      await expect(
        contract
          .connect(artist)
          ["safeTransferFrom(address,address,uint256)"](artist.address, otherArtist.address, 1)
      ).to.be.revertedWith("Pausable: paused");

      await contract.connect(deployer).unpause();

      await contract.connect(artist).transferFrom(artist.address, otherArtist.address, 1);
      expect(await contract.ownerOf(1)).to.equal(otherArtist.address);
    });

    it("leaves unrelated view functions working while paused", async function () {
      const { contract, deployer, artist, royaltyRecipient } = await loadFixture(deployFixture);

      const contentHash = hashOf("view-while-paused");
      await contract
        .connect(deployer)
        .mintCertificate(artist.address, "ipfs://v", contentHash, royaltyRecipient.address, 100);

      await contract.connect(deployer).pause();

      expect(await contract.ownerOf(1)).to.equal(artist.address);
      expect(await contract.tokenContentHash(1)).to.equal(contentHash);
      expect(await contract.tokenURI(1)).to.equal("ipfs://v");
    });
  });

  describe("Burning", function () {
    it("lets the token owner burn their token; ownerOf then reverts, but hashRegistered stays true and re-mint reverts", async function () {
      const { contract, deployer, artist, otherArtist, royaltyRecipient } = await loadFixture(
        deployFixture
      );

      const contentHash = hashOf("burn-me");
      await contract
        .connect(deployer)
        .mintCertificate(artist.address, "ipfs://burn", contentHash, royaltyRecipient.address, 100);

      await contract.connect(artist).burn(1);

      await expect(contract.ownerOf(1)).to.be.reverted;
      expect(await contract.hashRegistered(contentHash)).to.be.true;

      await expect(
        contract
          .connect(deployer)
          .mintCertificate(
            otherArtist.address,
            "ipfs://burn-again",
            contentHash,
            royaltyRecipient.address,
            100
          )
      ).to.be.revertedWith("PhotoCertificate: duplicate content hash");
    });
  });

  describe("supportsInterface", function () {
    it("returns true for ERC-721, ERC-2981, and AccessControl interface IDs", async function () {
      const { contract } = await loadFixture(deployFixture);

      expect(await contract.supportsInterface(INTERFACE_ID_ERC721)).to.be.true;
      expect(await contract.supportsInterface(INTERFACE_ID_ERC2981)).to.be.true;
      expect(await contract.supportsInterface(INTERFACE_ID_ACCESS_CONTROL)).to.be.true;
    });

    it("returns false for a bogus interface ID", async function () {
      const { contract } = await loadFixture(deployFixture);
      expect(await contract.supportsInterface("0xffffffff")).to.be.false;
    });
  });
});
