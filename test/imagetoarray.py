from PIL import Image
import tkinter as tk
from tkinter import filedialog

def image_to_pixel_array(image_path, width, height):
    # Load the image and convert it to RGBA (to handle transparency)
    img = Image.open(image_path).convert('RGBA')
    img = img.resize((width, height))
    
    # Create the byte array
    byte_array = []
    current_byte = 0
    bit_count = 0
    
    for y in range(height):
        for x in range(width):
            r, g, b, a = img.getpixel((x, y))  # Get pixel (RGBA)
            # Treat fully transparent pixels as white (0) and non-transparent as black (1)
            bit = 1 if a == 0 else 0  # 1 for black (opaque), 0 for white (transparent)
            
            # Shift the current byte and add the bit
            current_byte = (current_byte << 1) | bit
            bit_count += 1
            
            # If we have 8 bits, add the byte to the array
            if bit_count == 8:
                byte_array.append(current_byte)
                current_byte = 0
                bit_count = 0
                
    # If there are leftover bits, pad and add to the array
    if bit_count > 0:
        current_byte <<= (8 - bit_count)
        byte_array.append(current_byte)
    
    return byte_array

def select_image_file():
    # Open a Tkinter file dialog to select a PNG file
    root = tk.Tk()
    root.withdraw()  # Hide the main window
    file_path = filedialog.askopenfilename(
        title="Select a PNG image",
        filetypes=[("PNG files", "*.png")],
    )
    return file_path

# Prompt the user to select a PNG file
image_path = select_image_file()

# Check if the user selected a file
if image_path:
    # Convert the image to a pixel array
    pixel_array = image_to_pixel_array(image_path, 70, 70)

    # Format for C/C++ output
    c_array = "const unsigned char epd_bitmap_wifi0_70x70 [] PROGMEM = {\n\t"
    c_array += ', '.join(f'0x{byte:02x}' for byte in pixel_array)
    c_array += '\n};'

    # Print the resulting C/C++ array
    print(c_array)
else:
    print("No file selected.")
