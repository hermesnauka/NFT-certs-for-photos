package com.devpowers.nftcerts.watermark;

import com.fasterxml.jackson.databind.ObjectMapper;
import org.apache.commons.imaging.Imaging;
import org.apache.commons.imaging.common.ImageMetadata;
import org.apache.commons.imaging.formats.jpeg.JpegImageMetadata;
import org.apache.commons.imaging.formats.tiff.constants.TiffTagConstants;
import org.junit.jupiter.api.Test;

import javax.imageio.ImageIO;
import java.awt.image.BufferedImage;
import java.io.ByteArrayOutputStream;
import java.nio.charset.StandardCharsets;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

class MetadataWatermarkServiceTest {

    private final MetadataWatermarkService service = new MetadataWatermarkService(new ObjectMapper());

    @Test
    void embedsContentHashAndDidIntoJpegExif() throws Exception {
        byte[] jpeg = generateTinyJpeg();

        byte[] watermarked = service.watermark(jpeg, "image/jpeg", "abc123hash", "did:key:zArtist");

        assertTrue(watermarked.length > 0);

        ImageMetadata metadata = Imaging.getMetadata(watermarked);
        assertTrue(metadata instanceof JpegImageMetadata);
        JpegImageMetadata jpegMetadata = (JpegImageMetadata) metadata;

        String description = (String) jpegMetadata.getExif()
                .findField(TiffTagConstants.TIFF_TAG_IMAGE_DESCRIPTION)
                .getValue();

        assertTrue(description.contains("abc123hash"));
        assertTrue(description.contains("did:key:zArtist"));
    }

    @Test
    void nonJpegContentIsReturnedUnchanged() {
        byte[] pngLike = "not really an image".getBytes(StandardCharsets.UTF_8);

        byte[] result = service.watermark(pngLike, "image/png", "somehash", "did:key:zArtist");

        assertEquals(pngLike, result);
    }

    private byte[] generateTinyJpeg() throws Exception {
        BufferedImage image = new BufferedImage(4, 4, BufferedImage.TYPE_INT_RGB);
        ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
        ImageIO.write(image, "jpg", outputStream);
        return outputStream.toByteArray();
    }
}
