from PIL import Image

def rgb888_to_rgb565(r, g, b):
    """ Convert 24-bit RGB to 16-bit RGB565 """
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

# Load and resize image to 320x240
img = Image.open(r"C:\Users\Admin\Downloads\petch.jpg").resize((320, 240)).convert("RGB")

# Convert pixels to RGB565 format
pixels = list(img.getdata())
rgb565_data = [rgb888_to_rgb565(*pixel) for pixel in pixels]

# Write the C array
with open("image_data.c", "w") as f:
    f.write("const uint8_t snow_tiger[320*240*2] = {\n")
    for i, value in enumerate(rgb565_data):
        # Extract High & Low Bytes (Big Endian)
        high_byte = (value >> 8) & 0xFF
        low_byte = value & 0xFF
        f.write(f"0x{high_byte:02X}, 0x{low_byte:02X}, ")
        if (i + 1) % 8 == 0:
            f.write("\n")
    f.write("};\n")
