import os
import sys
from PIL import Image, ImageColor

def format_c_array(array, line_width=80):
    """
    Format a C array string with line breaks to ensure each line's width is within the limit.

    :param array: List of array elements as strings.
    :param line_width: Maximum width of each line.
    :return: Formatted C array string.
    """
    formatted_lines = []
    current_line = []

    current_length = 0
    for item in array:
        item_length = len(item) + 2  # Account for ", "
        if current_length + item_length > line_width:
            formatted_lines.append(", ".join(current_line))
            current_line = []
            current_length = 0
        current_line.append(item)
        current_length += item_length

    if current_line:
        formatted_lines.append(", ".join(current_line))

    return ",\n    ".join(formatted_lines)

def image_to_c_array(image, name, palette_size):
    """
    Convert an image to a C array representation.
    
    :param image: The image to convert (palette mode)
    :param name: The base name for the C arrays
    :param palette_size: The size of the palette (256 or 16)
    :return: Tuple of (palette_c_array, pixel_c_array)
    """
    # Extract palette and pixel data
    palette = image.getpalette()[:palette_size * 3]
    pixels = list(image.getdata())

    # Convert palette to 0x00RRGGBB format (RGB only, no alpha)
    palette_array = [
        f"0x{(palette[i] << 16 | palette[i + 1] << 8 | palette[i + 2]):06x}"
        for i in range(0, len(palette), 3)
    ]

    # Convert pixels to hex format
    if palette_size == 16:
        # Pack two 4-bit pixels into one byte
        packed_pixels = []
        for i in range(0, len(pixels), 2):
            pixel1 = pixels[i] & 0x0F
            pixel2 = pixels[i + 1] & 0x0F if i + 1 < len(pixels) else 0
            packed_byte = (pixel1 << 4) | pixel2
            packed_pixels.append(f"0x{packed_byte:02x}")
        pixel_array = packed_pixels
    else:
        # 256-color palette uses one byte per pixel
        pixel_array = [f"0x{pixel:02x}" for pixel in pixels]

    # 正确的命名逻辑
    if palette_size == 16:
        pal_suffix, pix_suffix = "pal4", "pix4"
    else:  # palette_size == 256
        pal_suffix, pix_suffix = "pal8", "pix8"
    
    palette_c_array = f"const uint32_t {name}_{pal_suffix}[] = {{\n    {format_c_array(palette_array)}\n}};"
    pixel_c_array = f"const uint8_t {name}_{pix_suffix}[] = {{\n    {format_c_array(pixel_array)}\n}};"

    return palette_c_array, pixel_c_array

def image_to_c_array_5bit_palette_3bit_alpha(image, alpha_data, name):
    """
    Convert image to 5-bit palette + 3-bit alpha combined format
    Each pixel: [7:5] = 3-bit alpha, [4:0] = 5-bit palette index
    """
    palette_size = 32
    palette = image.getpalette()[:palette_size * 3]
    pixels = list(image.getdata())
    
    # Quantize alpha to 3-bit (8 levels: 0, 36, 72, 108, 144, 180, 216, 255)
    alpha_3bit = [(a * 7 // 255) for a in alpha_data]  # Scale to 0-7
    
    # Combine: [7:5] = alpha, [4:0] = palette_index
    combined_pixels = []
    for i, pixel_idx in enumerate(pixels):
        alpha_val = alpha_3bit[i] & 0x07
        pixel_val = pixel_idx & 0x1F
        combined_byte = (alpha_val << 5) | pixel_val
        combined_pixels.append(f"0x{combined_byte:02x}")
    
    palette_array = [
        f"0x{(palette[i] << 16 | palette[i + 1] << 8 | palette[i + 2]):06x}"
        for i in range(0, len(palette), 3)
    ]

    palette_c_array = f"const uint32_t {name}_pal5[] = {{\n    {format_c_array(palette_array)}\n}};"
    pixel_c_array = f"const uint8_t {name}_pix5[] = {{\n    {format_c_array(combined_pixels)}\n}};"
    
    return palette_c_array, pixel_c_array

def image_to_c_array_24bit(image, name):
    """
    Convert an image to 24-bit RGB C array representation (no palette).
    
    :param image: The image to convert (RGB/RGBA mode)
    :param name: The base name for the C array
    :return: RGB pixel C array
    """
    # Convert to RGB if needed
    if image.mode != 'RGBA':
        rgb_image = image.convert('RGBA')
    else:
        rgb_image = image

    pixels = list(rgb_image.getdata())
    
    # Convert to 0x00RRGGBB format
    pixel_array = [f"0x{(a << 24 | r << 16 | g << 8 | b):08x}" for r, g, b, a in pixels]
    
    pixel_c_array = f"const uint32_t {name}_pix24[] = {{\n    {format_c_array(pixel_array)}\n}};"
    
    return pixel_c_array

def quantize_alpha(alpha_data, alpha_bits):
    """
    Quantize alpha data to specified bit depth
    
    :param alpha_data: List of alpha values (0-255)
    :param alpha_bits: Target bit depth (2, 4, or 8)
    :return: List of quantized alpha values
    """
    if alpha_bits == 2:
        # 2-bit: 4 levels (0, 85, 170, 255)
        quantized_alpha = []
        for a in alpha_data:
            if a < 64:
                quantized_alpha.append(0)      # 完全透明
            elif a < 128:
                quantized_alpha.append(85)     # 1/3透明
            elif a < 192:
                quantized_alpha.append(170)    # 2/3透明
            else:
                quantized_alpha.append(255)    # 完全不透明
        return quantized_alpha
    elif alpha_bits == 4:
        # 4-bit: 16 levels (0, 17, 34, ..., 255)
        return [(a >> 4) << 4 for a in alpha_data]
    else:  # alpha_bits == 8
        # 8-bit: keep original values
        return alpha_data

def generate_alpha_array(name, alpha_data, alpha_bits):
    """
    为所有模式生成alpha数组（支持2-bit, 4-bit, 8-bit打包）
    """
    if not alpha_data or len(alpha_data) == 0:
        return ""
    
    print(f"    Generating {alpha_bits}-bit alpha array for {name}")
    
    # 量化alpha数据
    quantized_alpha = quantize_alpha(alpha_data, alpha_bits)
    
    if alpha_bits == 2:
        # 将alpha值转换为2bit (0,1,2,3)
        alpha_2bit = []
        for a in quantized_alpha:
            if a == 0:
                alpha_2bit.append(0)
            elif a == 85:
                alpha_2bit.append(1)
            elif a == 170:
                alpha_2bit.append(2)
            else:  # 255
                alpha_2bit.append(3)
        
        # 2bit alpha打包：每字节4个alpha值
        packed_alpha = []
        for i in range(0, len(alpha_2bit), 4):
            byte_value = 0
            for j in range(4):
                if i + j < len(alpha_2bit):
                    alpha_value = alpha_2bit[i + j] & 0x03
                    byte_value |= (alpha_value << ((3 - j) * 2))
            packed_alpha.append(f"0x{byte_value:02x}")
        
        alpha_c_array = f"const uint8_t {name}_a{alpha_bits}[] = {{\n    {format_c_array(packed_alpha)}\n}};"
        print(f"    Alpha array generated with {len(packed_alpha)} bytes")
        
    elif alpha_bits == 4:
        # 将alpha值转换为4bit (0-15)
        alpha_4bit = [a >> 4 for a in quantized_alpha]
        
        # 4bit alpha打包：每字节2个alpha值
        packed_alpha = []
        for i in range(0, len(alpha_4bit), 2):
            alpha1 = alpha_4bit[i] & 0x0F
            alpha2 = alpha_4bit[i + 1] & 0x0F if i + 1 < len(alpha_4bit) else 0
            byte_value = (alpha1 << 4) | alpha2
            packed_alpha.append(f"0x{byte_value:02x}")
        
        alpha_c_array = f"const uint8_t {name}_a{alpha_bits}[] = {{\n    {format_c_array(packed_alpha)}\n}};"
        print(f"    Alpha array generated with {len(packed_alpha)} bytes")
        
    else:  # alpha_bits == 8
        # 8bit alpha：每字节1个alpha值
        alpha_array = [f"0x{a:02x}" for a in quantized_alpha]
        
        alpha_c_array = f"const uint8_t {name}_a{alpha_bits}[] = {{\n    {format_c_array(alpha_array)}\n}};"
        print(f"    Alpha array generated with {len(alpha_array)} bytes")
    
    return alpha_c_array

def process_single_frame(input_file, target_size, alpha_bits):
    """
    Process a single PNG file as one frame (square image)
    
    :param input_file: Path to input PNG file (single frame)
    :param target_size: Tuple specifying the target size for the frame (width, height)
    :param alpha_bits: Alpha channel bit depth (2, 4, or 8)
    :return: Dictionary containing processed image data
    """
    with Image.open(input_file) as img:
        # 总是转换为RGBA以保留alpha信息
        if img.mode != "RGBA":
            img = img.convert("RGBA")

        # 调整到目标大小
        frame = img.resize(target_size, Image.LANCZOS)
        
        # 从帧中提取alpha数据
        frame_pixels = list(frame.getdata())
        alpha_data = [a for r, g, b, a in frame_pixels]
        
        # 创建不透明版本用于调色板转换
        opaque_pixels = [(r, g, b, 255) for r, g, b, a in frame_pixels]
        opaque_frame = Image.new("RGBA", frame.size)
        opaque_frame.putdata(opaque_pixels)
        
        # 转换为RGB用于调色板生成
        rgb_version = opaque_frame.convert("RGB")
        palette_256_image = rgb_version.convert("P", palette=Image.ADAPTIVE, colors=256)
        palette_16_image = rgb_version.convert("P", palette=Image.ADAPTIVE, colors=16)
        palette_32_image = rgb_version.convert("P", palette=Image.ADAPTIVE, colors=32)
        
        return {
            'frame': frame,
            'palette_256_frame': palette_256_image,
            'palette_16_frame': palette_16_image,
            'palette_32_frame': palette_32_image,
            'alpha_data': alpha_data
        }

def process_multiple_frames(input_files, target_size, alpha_bits, save_png=False):
    """
    Process multiple PNG files as animation frames
    
    :param input_files: List of paths to input PNG files
    :param target_size: Tuple specifying the target size for each frame (width, height)
    :param alpha_bits: Alpha channel bit depth (2, 4, or 8)
    :param save_png: Whether to save processed PNG files
    :return: Dictionary containing processed image data
    """
    if not input_files:
        raise ValueError("No input files provided")
    
    frame_count = len(input_files)
    output_width = target_size[0]
    output_height = target_size[1] * frame_count
    
    # 创建输出图像来保存所有帧（纵向排列，用于生成C数组）
    output_image = Image.new("RGBA", (output_width, output_height), (0, 0, 0, 0))
    
    # 创建预览图像来保存所有帧（横向排列，用于预览）
    preview_width = target_size[0] * frame_count
    preview_height = target_size[1]
    preview_image = Image.new("RGBA", (preview_width, preview_height), (0, 0, 0, 0))
    
    all_alpha_data = []
    
    # 处理每一帧
    processed_frames = []
    for i, input_file in enumerate(input_files):
        print(f"    Processing frame {i+1}/{frame_count}: {os.path.basename(input_file)}")
        frame_data = process_single_frame(input_file, target_size, alpha_bits)
        processed_frames.append(frame_data)
        
        # 将帧添加到输出图像（纵向排列）
        output_upper = i * target_size[1]
        output_image.paste(frame_data['frame'], (0, output_upper))
        
        # 将帧添加到预览图像（横向排列）
        preview_left = i * target_size[0]
        preview_image.paste(frame_data['frame'], (preview_left, 0))
        
        # 收集alpha数据
        all_alpha_data.extend(frame_data['alpha_data'])
    
    # 创建组合的调色板图像
    final_pixels = list(output_image.getdata())
    opaque_pixels = [(r, g, b, 255) for r, g, b, a in final_pixels]
    opaque_image = Image.new("RGBA", output_image.size)
    opaque_image.putdata(opaque_pixels)
    
    rgb_version = opaque_image.convert("RGB")
    palette_256_image = rgb_version.convert("P", palette=Image.ADAPTIVE, colors=256)
    palette_16_image = rgb_version.convert("P", palette=Image.ADAPTIVE, colors=16)
    palette_32_image = rgb_version.convert("P", palette=Image.ADAPTIVE, colors=32)
    
    print(f"    Combined {frame_count} frames, total alpha data: {len(all_alpha_data)}")
    print(f"    Output image size: {output_image.size}")
    print(f"    Preview image size: {preview_image.size}")

    return {
        'output_image': output_image,
        'preview_image': preview_image,  # 新增：横向排列的预览图像
        'palette_256_image': palette_256_image,
        'palette_16_image': palette_16_image,
        'palette_32_image': palette_32_image,
        'has_alpha': True,
        'alpha_data': all_alpha_data,
        'alpha_bits': alpha_bits,
        'frame_count': frame_count
    }

def save_as_c_header(output_dir, name, all_images, alpha_bits, color_depth=4, target_size=(13, 13)):
    """
    Save all trail image data as a C header file with trail_res_t structure
    
    :param color_depth: Color depth (4, 8, or 24)
    :param target_size: Tuple specifying the target size for each frame (width, height)
    """
    name_clean = name.replace('-', '_')
    
    header_file = os.path.join(output_dir, f"{name}.h")
    
    # 生成带色深和alpha深度的宏后缀
    if color_depth == 24:
        suffix = "_c24"
    elif color_depth == 5:
        suffix = "_c5"
    else:
        suffix = f"_c{color_depth}_a{alpha_bits}"

    with open(header_file, "w") as f:
        f.write(f"#ifndef {name.upper()}{suffix.upper()}_H\n")
        f.write(f"#define {name.upper()}{suffix.upper()}_H\n\n")
        f.write(f"#include <stdint.h>\n\n")

        # Generate DEF macro for trail_res_t structure
        f.write(f"/* Trail definition macro */\n")
        f.write(f"#define DEF_{name}{suffix} {{ \\\n")

        # Generate each animation field according to trail_res_t structure
        trail_types = ['stem', 'marker_off', 'marker_on', 'marker_glow', 'arrow_tail', 'arrow', 'arrow_zoom']
        
        for i, trail_type in enumerate(trail_types):
            if trail_type in all_images:
                image_data = all_images[trail_type]
                frame_num = image_data['frame_count']
                alpha_data = image_data.get('alpha_data', None)
                
                print(f"    {trail_type}: color_depth={color_depth}, alpha_depth={alpha_bits}, frames={frame_num}")
                
                # 构建指针名称
                if color_depth == 24:
                    palette_ptr = "NULL"
                    img_ptr = f"{name_clean}_{trail_type}_pix24"
                elif color_depth == 5:
                    palette_ptr = f"{name_clean}_{trail_type}_pal5"
                    img_ptr = f"{name_clean}_{trail_type}_pix5"
                elif color_depth == 4:
                    palette_ptr = f"{name_clean}_{trail_type}_pal4"
                    img_ptr = f"{name_clean}_{trail_type}_pix4"
                else:  # color_depth == 8
                    palette_ptr = f"{name_clean}_{trail_type}_pal8"
                    img_ptr = f"{name_clean}_{trail_type}_pix8"
                
                # Alpha指针
                if color_depth == 5:
                    alpha_ptr = "NULL"
                elif alpha_data and len(alpha_data) > 0 and color_depth != 24:
                    alpha_ptr = f"{name_clean}_{trail_type}_a{alpha_bits}"
                else:
                    alpha_ptr = "NULL"
                
                # animation_t结构格式: color_depth, alpha_depth, image_size, frame_num, palette, alpha, img_union
                image_size = target_size[0]
                if color_depth == 24:
                    f.write(f"    .{trail_type} = {{ {color_depth}, 0, {image_size}, {frame_num}, {palette_ptr}, {alpha_ptr}, .img32 = {img_ptr} }}")
                elif color_depth == 5:
                    f.write(f"    .{trail_type} = {{ {color_depth}, 3, {image_size}, {frame_num}, {palette_ptr}, {alpha_ptr}, .img8 = {img_ptr} }}")
                else:
                    f.write(f"    .{trail_type} = {{ {color_depth}, {alpha_bits}, {image_size}, {frame_num}, {palette_ptr}, {alpha_ptr}, .img8 = {img_ptr} }}")
            else:
                f.write(f"    .{trail_type} = {{ 0, 0, 0, 0, NULL, NULL, .img8 = NULL }}")

            if i < len(trail_types) - 1:
                f.write(f", \\\n")
            else:
                f.write(f" \\\n")
        
        f.write(f"}}\n\n")

        # Write arrays for each trail type
        for trail_type, image_data in all_images.items():
            alpha_data = image_data.get('alpha_data', None)
            
            print(f"  Writing arrays for {trail_type}: depth={color_depth}")
            
            f.write(f"/* {trail_type.upper()} - Using {color_depth}-bit color depth")
            if color_depth == 5:
                f.write(f" with 3-bit alpha (combined)")
            elif alpha_data and len(alpha_data) > 0 and color_depth != 24:
                f.write(f" with {alpha_bits}-bit alpha")
            f.write(" */\n")
 
            if color_depth == 24:
                # 24-bit arrays (no palette)
                pixel_array = image_to_c_array_24bit(
                    image_data['output_image'], 
                    f"{name_clean}_{trail_type}"
                )
                f.write(f"{pixel_array}\n\n")
                
            elif color_depth == 5:
                # 5-bit palette + 3-bit alpha combined
                palette_array, pixel_array = image_to_c_array_5bit_palette_3bit_alpha(
                    image_data['palette_32_image'], 
                    alpha_data,
                    f"{name_clean}_{trail_type}"
                )
                f.write(f"{palette_array}\n\n")
                f.write(f"{pixel_array}\n\n")
                
            elif color_depth == 4:
                # 4-bit arrays
                palette_array, pixel_array = image_to_c_array(
                    image_data['palette_16_image'], 
                    f"{name_clean}_{trail_type}", 
                    16
                )
                f.write(f"{palette_array}\n\n")
                f.write(f"{pixel_array}\n\n")
                
            else:  # color_depth == 8
                # 8-bit arrays
                palette_array, pixel_array = image_to_c_array(
                    image_data['palette_256_image'], 
                    f"{name_clean}_{trail_type}", 
                    256
                )
                f.write(f"{palette_array}\n\n")
                f.write(f"{pixel_array}\n\n")
            
            # 生成alpha数组（如果有）
            if alpha_data and len(alpha_data) > 0 and color_depth not in [24, 5]:
                alpha_array = generate_alpha_array(f"{name_clean}_{trail_type}", alpha_data, alpha_bits)
                f.write(f"{alpha_array}\n\n")
        
        f.write(f"#endif // {name.upper()}{suffix.upper()}_H\n")

    print(f"Header file saved to {header_file}")

def process_trail_directory(input_dir, output_dir, frame_size=13, color_depth=4, alpha_bits=2, save_png=False):
    """
    Process LN0001_M*.png files in the input directory for trail animations
    
    :param input_dir: Input directory containing LN0001_M*.png files
    :param output_dir: Output directory for generated files
    :param frame_size: Target frame size in pixels
    :param color_depth: Color depth (4, 8, or 24)
    :param alpha_bits: Alpha channel bit depth (2, 4, or 8)
    :param save_png: Whether to save processed PNG files
    """
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # Define trail types and their expected file patterns
    trail_configs = {
        'stem': {'pattern': 'LN0001_M0', 'range': (1, 32)},      # M001-M032
        'marker_off': {'pattern': 'LN0001_M1', 'range': (0, 31)}, # M100-M131  
        'marker_on': {'pattern': 'LN0001_M2', 'range': (0, 31)},  # M200-M231
        'marker_glow': {'pattern': 'LN0001_M3', 'range': (0, 31)}, # M300-M331
        'arrow_tail': {'pattern': 'LN0001_M4', 'range': (0, 31)},  # M400-M431
        'arrow': {'pattern': 'LN0001_M5', 'range': (0, 31)},       # M500-M531
        'arrow_zoom': {'pattern': 'LN0001_M6', 'range': (0, 31)}   # M600-M631
    }

    target_size = (frame_size, frame_size)
    all_images = {}

    print(f"Processing trail directory: {input_dir}")

    for trail_type, config in trail_configs.items():
        # Look for files matching the pattern
        found_files = []
        pattern = config['pattern']
        start_num, end_num = config['range']
        
        # Collect all files with the pattern
        for i in range(start_num, end_num + 1):
            if trail_type == 'stem':
                # stem uses M001-M032
                filename = f"{pattern}{i:02d}.png"
            else:
                # others use M100-M131, M200-M231, etc.
                filename = f"{pattern}{i:02d}.png"
            
            filepath = os.path.join(input_dir, filename)
            if os.path.isfile(filepath):
                found_files.append(filepath)
        
        if found_files:
            print(f"  Processing {trail_type}: found {len(found_files)} files")
            
            try:
                # Sort files to ensure correct frame order
                found_files.sort()
                
                image_data = process_multiple_frames(
                    found_files,
                    target_size,
                    alpha_bits,
                    save_png
                )
                all_images[trail_type] = image_data
                print(f"    Successfully processed {trail_type} with {image_data['frame_count']} frames")
            except Exception as e:
                print(f"    Failed to process {trail_type}: {e}")
        else:
            print(f"  Warning: No files found for {trail_type} (pattern: {pattern})")

    # Generate C header file if any images were processed
    if all_images:
        base_name = f"trail{frame_size}"  # trail + frame_size, e.g., trail13
        print(f"  Using {color_depth}-bit color depth with {alpha_bits}-bit alpha")
        save_as_c_header(output_dir, base_name, all_images, alpha_bits, color_depth, target_size)
        
        # Save preview PNG files for each animation type
        if save_png:
            print(f"  Saving preview PNG files...")
            for trail_type, image_data in all_images.items():
                preview_file = os.path.join(output_dir, f"{base_name}_{trail_type}_preview.png")
                image_data['preview_image'].save(preview_file)
                print(f"    Saved preview: {preview_file}")
    else:
        print("No trail images were processed!")

if __name__ == "__main__":
    if len(sys.argv) < 3 or len(sys.argv) > 7:
        print("Usage: python3 trans_trail.py <input_directory> <output_directory> [frame_size] [color_depth] [alpha_bits] [png]")
        print("  frame_size: Frame size in pixels (default: 13)")
        print("  color_depth: 4, 5, 8, or 24 (default: 4)")
        print("  alpha_bits: 2, 4, or 8 (default: 2)")
        print("  Add 'png' at the end to save PNG files (default: only generate .h files)")
        print("Examples:")
        print("  python3 trans_trail.py input output")
        print("  python3 trans_trail.py input output 16")
        print("  python3 trans_trail.py input output 13 8")
        print("  python3 trans_trail.py input output 13 4 2")
        print("  python3 trans_trail.py input output 16 24 8 png")
        print("")
        print("Expected input files (each file is a single frame):")
        print("  LN0001_M001.png, LN0001_M002.png, ..., LN0001_M032.png (stem)")
        print("  LN0001_M100.png, LN0001_M101.png, ..., LN0001_M131.png (marker_off)")
        print("  LN0001_M200.png, LN0001_M201.png, ..., LN0001_M231.png (marker_on)")
        print("  LN0001_M300.png, LN0001_M301.png, ..., LN0001_M331.png (marker_glow)")
        print("  LN0001_M400.png, LN0001_M401.png, ..., LN0001_M431.png (arrow_tail)")
        print("  LN0001_M500.png, LN0001_M501.png, ..., LN0001_M531.png (arrow)")
        print("  LN0001_M600.png, LN0001_M601.png, ..., LN0001_M631.png (arrow_zoom)")
        sys.exit(1)

    input_dir = sys.argv[1]
    output_dir = sys.argv[2]
    
    # Parse parameters
    frame_size = 13  # default
    color_depth = 4  # default
    alpha_bits = 2  # default
    save_png = False
    
    # Parse remaining arguments
    remaining_args = sys.argv[3:]
    arg_index = 0
    
    for arg in remaining_args:
        if arg.lower() == 'png':
            save_png = True
        elif arg.isdigit() and arg_index == 0:  # frame_size
            frame_size = int(arg)
            arg_index += 1
        elif arg in ['4', '5', '8', '24'] and arg_index == 1:  # color_depth
            color_depth = int(arg)
            arg_index += 1
        elif arg in ['2', '4', '8'] and arg_index == 2:  # alpha_bits
            alpha_bits = int(arg)
            arg_index += 1
        else:
            print(f"Error: Invalid argument '{arg}' or arguments in wrong order.")
            print("Arguments order: [frame_size] [color_depth] [alpha_bits] [png]")
            print("Frame size must be a positive integer.")
            print("Color depth must be 4, 5, 8, or 24.")
            print("Alpha bits must be 2, 4, or 8.")
            sys.exit(1)

    if not os.path.isdir(input_dir):
        print(f"Error: {input_dir} is not a valid directory.")
        sys.exit(1)

    print(f"Processing with {frame_size}x{frame_size} frame size, {color_depth}-bit color depth and {alpha_bits}-bit alpha channel")
    print(f"Save PNG files: {'Yes' if save_png else 'No (only .h files)'}")
    
    process_trail_directory(input_dir, output_dir, frame_size, color_depth, alpha_bits, save_png)
