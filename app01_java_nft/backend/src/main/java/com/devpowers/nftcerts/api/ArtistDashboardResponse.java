package com.devpowers.nftcerts.api;

import java.util.List;

public record ArtistDashboardResponse(String walletAddress, List<CertificateDto> certificates, int totalCertificates) {
}
