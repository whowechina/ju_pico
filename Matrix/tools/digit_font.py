#!/usr/bin/env python3
"""
Digit Font Generator for Embedded UI
Generates anti-aliased digit bitmaps matching font_t structure
"""

import os
import sys
import argparse
from PIL import Image, ImageDraw, ImageFont

def find_all_fonts():
    """
    Find all system fonts
    
    :return: List of (font_path, font_name) tuples
    """
    all_fonts = []
    
    # Common font directories
    font_dirs = [
        "/System/Library/Fonts/",  # macOS
        "/System/Library/Fonts/Supplemental/",  # macOS additional
        "/usr/share/fonts/",       # Linux
        "/usr/share/fonts/truetype/",  # Linux truetype
        "/usr/local/share/fonts/", # Linux local
        "C:/Windows/Fonts/",       # Windows
    ]
    
    print("Scanning for fonts...")
    for font_dir in font_dirs:
        if os.path.exists(font_dir):
            print(f"  Scanning: {font_dir}")
            for root, dirs, files in os.walk(font_dir):
                for file in files:
                    if file.lower().endswith(('.ttf', '.ttc', '.otf')):
                        font_path = os.path.join(root, file)
                        font_name = os.path.splitext(file)[0]
                        all_fonts.append((font_path, font_name))
    
    print(f"Found {len(all_fonts)} font files")
    return all_fonts

def test_font_digits(font_path, test_size=100):
    """
    Test if font supports digits by comparing 0 and 1
    """
    try:
        font = ImageFont.truetype(font_path, test_size)
        
        # Create temporary image for measuring
        temp_img = Image.new('RGB', (200, 200), (0, 0, 0))
        draw = ImageDraw.Draw(temp_img)
        
        # Test digits 0 and 1 - they should be clearly different
        digit_data = {}
        
        for digit_char in ['0', '1']:
            try:
                # Get bounding box
                bbox = draw.textbbox((0, 0), digit_char, font=font)
                width = bbox[2] - bbox[0]
                height = bbox[3] - bbox[1]
                
                # Check if the digit actually rendered
                if width <= 0 or height <= 0:
                    return None
                
                # Render digit to bitmap
                digit_img = Image.new('RGB', (width + 4, height + 4), (0, 0, 0))
                digit_draw = ImageDraw.Draw(digit_img)
                digit_draw.text((2, 2), digit_char, fill=(255, 255, 255), font=font)
                
                # Convert to grayscale and calculate signature
                digit_gray = digit_img.convert('L')
                pixels = list(digit_gray.getdata())
                signature = sum(p for p in pixels if p > 20)  # Sum of bright pixels
                
                if signature == 0:
                    return None  # Empty rendering
                
                digit_data[digit_char] = {
                    'width': width,
                    'height': height,
                    'signature': signature
                }
                
            except Exception:
                return None
        
        # Check if we got both digits
        if len(digit_data) != 2:
            return None
        
        # Compare 0 and 1 - they should be different
        sig0 = digit_data['0']['signature']
        sig1 = digit_data['1']['signature']
        
        if sig0 > 0 and sig1 > 0:
            diff_ratio = abs(sig0 - sig1) / max(sig0, sig1)
            if diff_ratio < 0.1:  # Less than 10% difference means they're too similar
                return None
        
        # Calculate average dimensions
        avg_width = (digit_data['0']['width'] + digit_data['1']['width']) / 2
        avg_height = (digit_data['0']['height'] + digit_data['1']['height']) / 2
        aspect_ratio = avg_height / avg_width if avg_width > 0 else 0
        
        return (avg_width, avg_height, aspect_ratio, True)
        
    except Exception:
        return None

def is_likely_symbol_font(font_name):
    """
    Check if font is likely a symbol/icon font that shouldn't contain normal digits
    """
    symbol_keywords = [
        'symbol', 'icon', 'ding', 'wingding', 'webding', 'emoji', 'braille',
        'math', 'musical', 'hieroglyph', 'ornament', 'decorative', 'sign',
        'pictogram', 'glyph', 'geometric', 'supplement'
    ]
    
    font_name_lower = font_name.lower()
    return any(keyword in font_name_lower for keyword in symbol_keywords)

def load_font(font_path, size):
    """
    Load font with error handling
    """
    try:
        font = ImageFont.truetype(font_path, size)
        font.path = font_path
        return font
    except Exception as e:
        return None

def render_digit(digit, font, width, height, bg_color=(0, 0, 0), fg_color=(255, 255, 255)):
    """
    Render a single digit with anti-aliasing, maximizing use of available space
    """
    # Create image with higher resolution for better anti-aliasing
    scale = 4
    img = Image.new('RGB', (width * scale, height * scale), bg_color)
    draw = ImageDraw.Draw(img)
    
    # Get text bounding box
    bbox = draw.textbbox((0, 0), digit, font=font)
    text_width = bbox[2] - bbox[0]
    text_height = bbox[3] - bbox[1]
    
    # Calculate scaling factor to maximize font usage
    # Leave small margin (5% on each side)
    margin_x = width * scale * 0.05
    margin_y = height * scale * 0.05
    
    available_width = width * scale - 2 * margin_x
    available_height = height * scale - 2 * margin_y
    
    # Calculate scale factors for both dimensions
    scale_x = available_width / text_width if text_width > 0 else 1
    scale_y = available_height / text_height if text_height > 0 else 1
    
    # Use the smaller scale to ensure text fits in both dimensions
    text_scale = min(scale_x, scale_y)
    
    # Create a larger font if needed
    if text_scale > 1.0:
        # Calculate new font size
        current_font_size = font.size
        new_font_size = int(current_font_size * text_scale * 0.95)  # for safety margin
        
        try:
            # Try to load larger font
            larger_font = load_font(font.path if hasattr(font, 'path') else None, new_font_size)
            if larger_font:
                # Recalculate bbox with larger font
                bbox = draw.textbbox((0, 0), digit, font=larger_font)
                text_width = bbox[2] - bbox[0]
                text_height = bbox[3] - bbox[1]
                font = larger_font
        except:
            # If larger font fails, stick with original
            pass
    
    # Center the text
    x = (width * scale - text_width) // 2 - bbox[0]  # Adjust for bbox offset
    y = (height * scale - text_height) // 2 - bbox[1]  # Adjust for bbox offset
    
    # Draw the text
    draw.text((x, y), digit, fill=fg_color, font=font)
    
    # Scale down with anti-aliasing
    img = img.resize((width, height), Image.LANCZOS)
    
    return img

def image_to_c_array(img, bit_depth):
    """
    Convert PIL Image to C uint8_t array with specified bit depth
    """
    pixels = list(img.getdata())
    width, height = img.size
    
    if bit_depth == 2:
        # 2-bit: 4 pixels per byte
        pixels_per_byte = 4
        max_value = 3
    elif bit_depth == 4:
        # 4-bit: 2 pixels per byte
        pixels_per_byte = 2
        max_value = 15
    else:
        raise ValueError(f"Unsupported bit depth: {bit_depth}")
    
    # Convert RGB to grayscale and quantize
    gray_pixels = []
    for r, g, b in pixels:
        gray = int(0.299 * r + 0.587 * g + 0.114 * b)
        quantized = (gray * max_value) // 255
        gray_pixels.append(quantized)
    
    # Pack pixels - 使用简单的索引计算
    packed_data = []
    total_pixels = width * height
    total_bytes = (total_pixels + pixels_per_byte - 1) // pixels_per_byte  # 向上取整
    
    for byte_idx in range(total_bytes):
        byte_value = 0
        for i in range(pixels_per_byte):
            pixel_idx = byte_idx * pixels_per_byte + i
            if pixel_idx < total_pixels:
                pixel_value = gray_pixels[pixel_idx]
                if bit_depth == 2:
                    byte_value |= (pixel_value << ((3 - i) * 2))
                elif bit_depth == 4:
                    byte_value |= (pixel_value << ((1 - i) * 4))
        packed_data.append(byte_value)
    
    return packed_data, width

def generate_font_for_single_font(font_path, font_name, height, max_width, bit_depth, output_dir):
    """
    Generate digit font for a single font
    """
    # Skip likely symbol fonts
    if is_likely_symbol_font(font_name):
        return False, "Likely symbol font"
    
    # Test font first - check digit support and dimensions
    result = test_font_digits(font_path)
    if not result:
        return False, "No digit support or identical digits"
    
    width, test_height, aspect_ratio, has_digits = result
    calculated_width = max(1, min(height, int(height / aspect_ratio)))
    
    # Check if calculated width exceeds the maximum allowed width
    if calculated_width > max_width:
        return False, f"Width {calculated_width} > max {max_width}"
    
    # Clean font name for filename
    clean_font_name = "".join(c for c in font_name if c.isalnum() or c in ('-', '_')).lower()
    output_file = os.path.join(output_dir, f"digit_font_{clean_font_name}_{calculated_width}x{height}_{bit_depth}bit.h")
    
    # Load font with initial size
    font = load_font(font_path, height)
    if not font:
        return False, "Font loading failed"
    
    try:
        # Render all digits and collect data
        all_digit_data = []
        preview_img = Image.new('RGB', (calculated_width, height * 10), (0, 0, 0))
        
        for digit in range(10):
            digit_char = str(digit)
            
            # Render digit with auto-scaling
            img = render_digit(digit_char, font, calculated_width, height)
            
            # Add to combined preview image
            preview_img.paste(img, (0, digit * height))
            
            # Convert to packed data
            packed_data, packed_width = image_to_c_array(img, bit_depth)
            all_digit_data.extend(packed_data)
        
        # Calculate total data size
        bytes_per_char = len(packed_data)
        total_bytes = len(all_digit_data)
        
        # Generate header file
        header_guard = os.path.basename(output_file).upper().replace('.', '_')
        var_name = f"digit_font_{clean_font_name}_{calculated_width}x{height}_{bit_depth}bit"
        macro_name = f"DEF_{var_name}"
        
        with open(output_file, 'w') as f:
            f.write(f"#ifndef {header_guard}\n")
            f.write(f"#define {header_guard}\n\n")
            f.write(f"#include \"font.h\"\n\n")
            
            f.write(f"/* Generated digit font: {font_name} */\n")
            f.write(f"/* Font file: {os.path.basename(font_path)} */\n")
            f.write(f"/* Aspect ratio: {aspect_ratio:.2f} */\n")
            f.write(f"/* Character size: {calculated_width}x{height} pixels */\n")
            f.write(f"/* Bit depth: {bit_depth} bits per pixel */\n")
            f.write(f"/* Data size: {total_bytes} bytes */\n\n")
            
            # Generate macro definition at the top
            f.write(f"#define {macro_name} {{ \\\n")
            f.write(f"    .width = {calculated_width}, \\\n")
            f.write(f"    .height = {height}, \\\n")
            f.write(f"    .first_char = '0', \\\n")
            f.write(f"    .char_count = 10, \\\n")
            f.write(f"    .bit_per_pixel = {bit_depth}, \\\n")
            f.write(f"    .data = {var_name}_data \\\n")
            f.write(f"}}\n\n")
            
            # Generate font data array
            f.write(f"static const uint8_t {var_name}_data[{total_bytes}] = {{\n")
            
            # Write data in rows of 16 bytes
            for i in range(0, len(all_digit_data), 16):
                row = all_digit_data[i:i+16]
                hex_values = [f"0x{b:02x}" for b in row]
                f.write(f"    {', '.join(hex_values)}")
                if i + 16 < len(all_digit_data):
                    f.write(",")
                f.write(f"\n")
            
            f.write(f"}};\n\n")
            
            # Generate font_t structure (optional, for direct use)
            f.write(f"const font_t {var_name} = {macro_name};\n\n")
            
            f.write(f"#endif // {header_guard}\n")
        
        # Save combined preview image
        preview_file = output_file.replace('.h', '_preview.png')
        preview_img.save(preview_file)
        
        # Calculate space savings vs 8-bit
        original_size = calculated_width * height * 10  # 8-bit
        actual_size = total_bytes
        savings = ((original_size - actual_size) / original_size) * 100 if original_size > 0 else 0
        
        return True, f"Generated {calculated_width}x{height} {bit_depth}bit, saved {savings:.1f}% space ({actual_size} vs {original_size} bytes)"
        
    except Exception as e:
        return False, f"Generation error: {e}"

def generate_all_fonts(height, max_width, bit_depths, output_dir):
    """
    Generate digit fonts for ALL system fonts that meet width criteria
    """
    # Create output directory if it doesn't exist
    if not os.path.exists(output_dir):
        print(f"Creating output directory: {output_dir}")
        os.makedirs(output_dir)
    
    # Find all fonts
    all_fonts = find_all_fonts()
    
    if not all_fonts:
        print("No fonts found!")
        return
    
    print(f"\nGenerating digit fonts for {len(all_fonts)} fonts...")
    print(f"Height: {height} pixels")
    print(f"Max width: {max_width} pixels")
    print(f"Bit depths: {bit_depths}")
    print(f"Output directory: {output_dir}\n")
    
    total_successful = 0
    total_failed = 0
    
    # Generate fonts for all fonts and bit depths
    for bit_depth in bit_depths:
        print(f"\n=== Generating {bit_depth}-bit fonts ===")
        successful = 0
        failed = 0
        skipped_width = 0
        skipped_digits = 0
        
        for i, (font_path, font_name) in enumerate(all_fonts):
            print(f"[{i+1:3d}/{len(all_fonts)}] Processing: {font_name}")
            
            success, reason = generate_font_for_single_font(font_path, font_name, height, max_width, bit_depth, output_dir)
            
            if success:
                successful += 1
                total_successful += 1
                print(f"    ✓ {reason}")
            else:
                failed += 1
                total_failed += 1
                if "Width" in reason and "max" in reason:
                    skipped_width += 1
                    print(f"    ⚠ Skipped: {reason}")
                elif "No digit support" in reason:
                    skipped_digits += 1
                    print(f"    ⚠ Skipped: {reason}")
                else:
                    print(f"    ✗ Failed: {reason}")
        
        print(f"\n{bit_depth}-bit Summary:")
        print(f"  Successfully generated: {successful}")
        print(f"  Skipped (width > {max_width}): {skipped_width}")
        print(f"  Skipped (no digit support): {skipped_digits}")
        print(f"  Failed (other errors): {failed - skipped_width - skipped_digits}")
    
    print(f"\n=== Final Summary ===")
    print(f"Total fonts processed: {len(all_fonts) * len(bit_depths)}")
    print(f"Total successful: {total_successful}")
    print(f"Total failed: {total_failed}")
    print(f"Output directory: {output_dir}")

def main():
    parser = argparse.ArgumentParser(description='Generate anti-aliased digit fonts matching font_t structure')
    parser.add_argument('height', type=int, help='Character height in pixels')
    parser.add_argument('max_width', type=int, help='Maximum allowed character width in pixels')
    parser.add_argument('output_dir', help='Output directory for generated fonts')
    parser.add_argument('--bit-depths', nargs='+', type=int, choices=[2, 4], default=[2, 4],
                       help='Bit depths to generate (default: 2 4)')
    
    args = parser.parse_args()
    
    # Validate arguments
    if args.height <= 0:
        print("Error: Height must be a positive integer")
        sys.exit(1)
    
    if args.max_width <= 0:
        print("Error: Max width must be a positive integer")
        sys.exit(1)
    
    # Generate fonts for all suitable system fonts
    try:
        generate_all_fonts(args.height, args.max_width, args.bit_depths, args.output_dir)
    except Exception as e:
        print(f"Error generating fonts: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()