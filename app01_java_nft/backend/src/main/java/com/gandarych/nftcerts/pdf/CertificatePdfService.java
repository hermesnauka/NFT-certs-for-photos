package com.gandarych.nftcerts.pdf;

import com.gandarych.nftcerts.certificate.Artwork;
import com.gandarych.nftcerts.certificate.Certificate;
import com.lowagie.text.Document;
import com.lowagie.text.Element;
import com.lowagie.text.Font;
import com.lowagie.text.PageSize;
import com.lowagie.text.Paragraph;
import com.lowagie.text.Phrase;
import com.lowagie.text.pdf.PdfPCell;
import com.lowagie.text.pdf.PdfPTable;
import com.lowagie.text.pdf.PdfWriter;
import org.springframework.stereotype.Service;

import java.io.ByteArrayOutputStream;
import java.time.format.DateTimeFormatter;

/**
 * Renders a one-page, downloadable digital certificate of authenticity for a minted NFT
 * certificate, using OpenPDF.
 */
@Service
public class CertificatePdfService {

    private static final Font TITLE_FONT = new Font(Font.HELVETICA, 22, Font.BOLD);
    private static final Font HEADING_FONT = new Font(Font.HELVETICA, 13, Font.BOLD);
    private static final Font LABEL_FONT = new Font(Font.HELVETICA, 10, Font.BOLD);
    private static final Font VALUE_FONT = new Font(Font.HELVETICA, 10, Font.NORMAL);
    private static final Font BODY_FONT = new Font(Font.HELVETICA, 10, Font.ITALIC);

    public byte[] generate(Certificate certificate, String etherscanUrl, String openSeaUrl, String raribleUrl) {
        Artwork artwork = certificate.getArtwork();
        ByteArrayOutputStream outputStream = new ByteArrayOutputStream();

        try {
            Document document = new Document(PageSize.A4, 50, 50, 50, 50);
            PdfWriter.getInstance(document, outputStream);
            document.open();

            document.add(new Paragraph("Certificate of Authenticity", TITLE_FONT));
            document.add(spacer());
            document.add(new Paragraph(nullToDash(artwork == null ? null : artwork.getTitle()), HEADING_FONT));
            document.add(spacer());

            PdfPTable table = new PdfPTable(2);
            table.setWidthPercentage(100);
            table.setWidths(new float[]{1f, 2f});

            addRow(table, "Token ID", String.valueOf(certificate.getTokenId()));
            addRow(table, "Artist Wallet", nullToDash(certificate.getOwnerAddress()));
            addRow(table, "Artist DID", nullToDash(artwork == null ? null : artwork.getArtistDid()));
            addRow(table, "Content Hash (SHA-256)", nullToDash(certificate.getContentHashHex()));
            addRow(table, "Medium", nullToDash(artwork == null ? null : artwork.getMedium()));
            addRow(table, "Year Created", artwork == null || artwork.getYearCreated() == null
                    ? "-" : String.valueOf(artwork.getYearCreated()));
            addRow(table, "Royalty", formatBps(certificate.getRoyaltyPercentageBps()));
            addRow(table, "Royalty Recipient", nullToDash(certificate.getRoyaltyRecipient()));
            addRow(table, "Contract Address", nullToDash(certificate.getContractAddress()));
            addRow(table, "Image (IPFS)", nullToDash(artwork == null ? null : artwork.getImageIpfsUri()));
            addRow(table, "Metadata (IPFS)", nullToDash(artwork == null ? null : artwork.getMetadataIpfsUri()));
            addRow(table, "Transaction", nullToDash(etherscanUrl));
            addRow(table, "OpenSea", nullToDash(openSeaUrl));
            addRow(table, "Rarible", nullToDash(raribleUrl));
            addRow(table, "Minted At", certificate.getMintedAt() == null ? "-"
                    : DateTimeFormatter.ISO_INSTANT.format(certificate.getMintedAt()));

            document.add(table);
            document.add(spacer());

            document.add(new Paragraph(
                    "This certificate attests that the photograph identified above has been cryptographically "
                            + "fingerprinted (SHA-256) and permanently recorded on a public blockchain as a "
                            + "non-fungible token, together with its metadata pinned to IPFS. The recorded "
                            + "content hash and on-chain royalty configuration (EIP-2981) allow any marketplace "
                            + "or third party to verify the artwork's authenticity and the creator's right to "
                            + "royalties on secondary sales.",
                    BODY_FONT));

            document.close();
            return outputStream.toByteArray();
        } catch (com.lowagie.text.DocumentException e) {
            throw new IllegalStateException("Failed to generate certificate PDF", e);
        }
    }

    private void addRow(PdfPTable table, String label, String value) {
        PdfPCell labelCell = new PdfPCell(new Phrase(label, LABEL_FONT));
        labelCell.setBorder(0);
        labelCell.setPaddingBottom(6);
        table.addCell(labelCell);

        PdfPCell valueCell = new PdfPCell(new Phrase(value, VALUE_FONT));
        valueCell.setBorder(0);
        valueCell.setPaddingBottom(6);
        table.addCell(valueCell);
    }

    private Paragraph spacer() {
        Paragraph paragraph = new Paragraph(" ");
        paragraph.setAlignment(Element.ALIGN_LEFT);
        return paragraph;
    }

    private String formatBps(Integer bps) {
        if (bps == null) {
            return "-";
        }
        return (bps / 100.0) + "%";
    }

    private String nullToDash(String value) {
        return value == null || value.isBlank() ? "-" : value;
    }
}
