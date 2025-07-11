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

def quantize_image_advanced(image, target_mode, palette_size, method='adaptive'):
    """
    高级调色板量化函数
    
    :param method: 'adaptive', 'octree', 'kmeans', 'wu', 'libimagequant'
    """
    if target_mode != 'P':
        raise ValueError(f"Unsupported target mode: {target_mode}")
    
    if method == 'adaptive':
        # 当前的快速方法
        return image.convert("P", palette=Image.ADAPTIVE, colors=palette_size)
    
    elif method == 'octree':
        # 更好的质量
        return image.quantize(colors=palette_size, method=Image.MAXCOVERAGE)
    
    elif method == 'wu':
        # 最佳质量（如果支持）
        try:
            return image.quantize(colors=palette_size, method=Image.FASTOCTREE)
        except:
            raise ValueError(f"Unsupported quantize method: {method}")
    
    elif method == 'kmeans':
        return quantize_kmeans(image, palette_size)
    
    elif method == 'libimagequant':
        return quantize_libimagequant(image, palette_size)
    
    else:
        # 默认方法
        return image.convert("P", palette=Image.ADAPTIVE, colors=palette_size)

def process_image_from_files(input_dir, image_type, target_size=(13, 13), alpha_bits=2, save_png=False):
    """
    从独立文件中处理图像（30fps版本）
    
    :param input_dir: 输入目录
    :param image_type: 图像类型 ('approach', 'miss', 'poor', 'good', 'great', 'perfect')
    :param target_size: 目标尺寸
    :param alpha_bits: Alpha通道位深
    :param save_png: 是否保存PNG
    """
    # 定义文件名模式和帧数
    if image_type == 'approach':
        file_pattern = "ma{:02d}.png"
        frame_range = range(0, 16)  # ma00.png - ma15.png，16帧
    elif image_type == 'miss':
        file_pattern = "ma{:02d}.png" 
        frame_range = range(16, 24)  # ma16.png - ma23.png，8帧
    elif image_type == 'poor':
        file_pattern = "h1{:02d}.png"
        frame_range = range(0, 16)  # h100.png - h115.png，16帧
    elif image_type == 'good':
        file_pattern = "h2{:02d}.png"
        frame_range = range(0, 16)  # h200.png - h215.png，16帧
    elif image_type == 'great':
        file_pattern = "h3{:02d}.png"
        frame_range = range(0, 16)  # h300.png - h315.png，16帧
    elif image_type == 'perfect':
        file_pattern = "h4{:02d}.png"
        frame_range = range(0, 16)  # h400.png - h415.png，16帧
    else:
        raise ValueError(f"Unknown image type: {image_type}")

    # 计算输出图像尺寸
    frame_count = len(frame_range)
    output_width = target_size[0]
    output_height = target_size[1] * frame_count
    
    # 创建输出图像
    output_image = Image.new("RGBA", (output_width, output_height), (0, 0, 0, 0))
    
    # 处理每个帧文件
    for i, frame_idx in enumerate(frame_range):
        if image_type in ['poor', 'good', 'great', 'perfect']:
            # h1xx, h2xx, h3xx, h4xx格式
            filename = file_pattern.format(frame_idx)
        else:
            # maxx格式
            filename = file_pattern.format(frame_idx)
            
        file_path = os.path.join(input_dir, filename)
        
        if not os.path.isfile(file_path):
            print(f"Warning: File not found: {file_path}")
            continue
            
        try:
            with Image.open(file_path) as frame_img:
                # 转换为RGBA
                if frame_img.mode != "RGBA":
                    frame_img = frame_img.convert("RGBA")
                
                # 缩放到目标尺寸
                frame_img = frame_img.resize(target_size, Image.LANCZOS)
                
                # 粘贴到输出图像
                output_upper = i * target_size[1]
                output_image.paste(frame_img, (0, output_upper))
                
        except Exception as e:
            print(f"Error processing {file_path}: {e}")
            continue
    
    # 提取alpha数据
    final_pixels = list(output_image.getdata())
    alpha_data = [a for r, g, b, a in final_pixels]
    
    # 创建不透明版本用于调色板转换
    opaque_pixels = [(r, g, b, 255) for r, g, b, a in final_pixels]
    opaque_image = Image.new("RGBA", output_image.size)
    opaque_image.putdata(opaque_pixels)
    
    # 转换为RGB用于调色板生成
    rgb_version = opaque_image.convert("RGB")
    palette_256_image = quantize_image_advanced(rgb_version, 'P', 256, 'wu')
    palette_16_image = quantize_image_advanced(rgb_version, 'P', 16, 'wu')
    palette_32_image = quantize_image_advanced(rgb_version, 'P', 32, 'wu')
    
    print(f"    Processed {frame_count} frames for {image_type}")
    print(f"    Alpha data length: {len(alpha_data)}, using {alpha_bits}-bit depth")
    print(f"    Image size: {output_image.size}, total pixels: {output_image.size[0] * output_image.size[1]}")
    
    return {
        'output_image': output_image,
        'palette_256_image': palette_256_image,
        'palette_16_image': palette_16_image,
        'palette_32_image': palette_32_image,
        'has_alpha': True,
        'alpha_data': alpha_data,
        'alpha_bits': alpha_bits,
        'frame_count': frame_count
    }

def save_as_c_header(output_dir, name, all_images, alpha_bits, color_depth=4):
    """
    保存30fps版本的C头文件
    """
    # 将文件名中的连字符替换为下划线，用于C标识符
    c_name = name.replace('-', '_')
    
    header_file = os.path.join(output_dir, f"{name}.h")  # 文件名保持原样
    
    # 生成带色深和alpha深度的宏后缀
    if color_depth == 24:
        suffix = "_c24"
    elif color_depth == 5:
        suffix = "_c5"
    else:
        suffix = f"_c{color_depth}_a{alpha_bits}"

    with open(header_file, "w") as f:
        f.write(f"#ifndef {c_name.upper()}{suffix.upper()}_H\n")
        f.write(f"#define {c_name.upper()}{suffix.upper()}_H\n\n")
        f.write(f"#include <stdint.h>\n\n")

        # Generate DEF macro at the top
        f.write(f"/* Marker definition macro - 30fps */\n")
        f.write(f"#define DEF_{c_name}{suffix} {{ \\\n")
        f.write(f"    .fps = 30, \\\n")  # 30fps
        f.write(f"    {{ \\\n")
        # Generate each animation field
        animation_types = ['approach', 'perfect', 'great', 'good', 'poor', 'miss']
        for i, anim_type in enumerate(animation_types):
            if anim_type in all_images:
                # 使用实际的帧数
                frame_num = all_images[anim_type]['frame_count']
                alpha_data = all_images[anim_type].get('alpha_data', None)
                
                print(f"    {anim_type}: color_depth={color_depth}, alpha_depth={alpha_bits}, frames={frame_num}")
                
                # 构建指针名称 - 使用c_name
                if color_depth == 24:
                    palette_ptr = "NULL"
                    img_ptr = f"{c_name}_{anim_type}_pix24"
                elif color_depth == 5:
                    palette_ptr = f"{c_name}_{anim_type}_pal5"
                    img_ptr = f"{c_name}_{anim_type}_pix5"
                elif color_depth == 4:
                    palette_ptr = f"{c_name}_{anim_type}_pal4"
                    img_ptr = f"{c_name}_{anim_type}_pix4"
                else:  # color_depth == 8
                    palette_ptr = f"{c_name}_{anim_type}_pal8"
                    img_ptr = f"{c_name}_{anim_type}_pix8"
                
                # Alpha指针
                if color_depth == 5:
                    alpha_ptr = "NULL"
                elif alpha_data and len(alpha_data) > 0 and color_depth != 24:
                    alpha_ptr = f"{c_name}_{anim_type}_a{alpha_bits}"
                else:
                    alpha_ptr = "NULL"
                
                # 结构格式
                if color_depth == 24:
                    f.write(f"        {{ {color_depth}, 0, 13, {frame_num}, {palette_ptr}, {alpha_ptr}, .img32 = {img_ptr} }}")
                elif color_depth == 5:
                    f.write(f"        {{ {color_depth}, 3, 13, {frame_num}, {palette_ptr}, {alpha_ptr}, .img8 = {img_ptr} }}")
                else:
                    f.write(f"        {{ {color_depth}, {alpha_bits}, 13, {frame_num}, {palette_ptr}, {alpha_ptr}, .img8 = {img_ptr} }}")
            else:
                f.write(f"        {{ 0, 0, 0, 0, NULL, NULL, .img8 = NULL }}")
            
            if i < len(animation_types) - 1:
                f.write(f", \\\n")
            else:
                f.write(f" \\\n")
        
        f.write(f"    }} \\\n")
        f.write(f"}}\n\n")

        # Write arrays for each image type
        for image_type, image_data in all_images.items():
            alpha_data = image_data.get('alpha_data', None)
            
            print(f"  Writing arrays for {image_type}: depth={color_depth}")
            
            f.write(f"/* {image_type.upper()} - Using {color_depth}-bit color depth")
            if color_depth == 5:
                f.write(f" with 3-bit alpha (combined)")
            elif alpha_data and len(alpha_data) > 0 and color_depth != 24:
                f.write(f" with {alpha_bits}-bit alpha")
            f.write(" */\n")
 
            if color_depth == 24:
                pixel_array = image_to_c_array_24bit(
                    image_data['output_image'], 
                    f"{c_name}_{image_type}"  # 使用c_name
                )
                f.write(f"{pixel_array}\n\n")
                
            elif color_depth == 5:
                palette_array, pixel_array = image_to_c_array_5bit_palette_3bit_alpha(
                    image_data['palette_32_image'], 
                    alpha_data,
                    f"{c_name}_{image_type}"  # 使用c_name
                )
                f.write(f"{palette_array}\n\n")
                f.write(f"{pixel_array}\n\n")
                
            elif color_depth == 4:
                palette_array, pixel_array = image_to_c_array(
                    image_data['palette_16_image'], 
                    f"{c_name}_{image_type}",  # 使用c_name
                    16
                )
                f.write(f"{palette_array}\n\n")
                f.write(f"{pixel_array}\n\n")
                
            else:  # color_depth == 8
                palette_array, pixel_array = image_to_c_array(
                    image_data['palette_256_image'], 
                    f"{c_name}_{image_type}",  # 使用c_name
                    256
                )
                f.write(f"{palette_array}\n\n")
                f.write(f"{pixel_array}\n\n")
            
            # 生成alpha数组（如果有）
            if alpha_data and len(alpha_data) > 0 and color_depth not in [24, 5]:
                alpha_array = generate_alpha_array(f"{c_name}_{image_type}", alpha_data, alpha_bits)  # 使用c_name
                f.write(f"{alpha_array}\n\n")
        
        f.write(f"#endif // {c_name.upper()}{suffix.upper()}_H\n")

    print(f"Header file saved to {header_file}")

def process_all_subdirectories(input_dir, output_dir, target_size=(13, 13), alpha_bits=2, color_depth=4, save_png=False):
    """
    处理30fps版本的所有子目录
    """
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    animation_types = ['approach', 'perfect', 'great', 'good', 'poor', 'miss']

    for subdir in os.listdir(input_dir):
        subdir_path = os.path.join(input_dir, subdir)
        if os.path.isdir(subdir_path):
            all_images = {}
            
            print(f"Processing directory: {subdir}")
            
            for image_type in animation_types:
                print(f"  Processing {image_type}...")
                try:
                    image_data = process_image_from_files(
                        subdir_path,
                        image_type,
                        target_size,
                        alpha_bits,
                        save_png
                    )
                    all_images[image_type] = image_data
                except Exception as e:
                    print(f"Failed to process {image_type}: {e}")

            # Generate C header file if any images were processed
            if all_images:
                print(f"  Using {color_depth}-bit color depth with {alpha_bits}-bit alpha (30fps)")
                save_as_c_header(output_dir, subdir, all_images, alpha_bits, color_depth)

if __name__ == "__main__":
    if len(sys.argv) < 3 or len(sys.argv) > 7:
        print("Usage: python3 trans_marker30.py <input_directory> <output_directory> [color_depth] [alpha_bits] [png]")
        print("  color_depth: 4, 5, 8, or 24 (default: 4)")
        print("  alpha_bits: 2, 4, or 8 (default: 2)")
        print("  Add 'png' at the end to save PNG files (default: only generate .h files)")
        print("30fps version - frames are halved and read from individual files")
        print("Examples:")
        print("  python3 trans_marker30.py input output")
        print("  python3 trans_marker30.py input output 8")
        print("  python3 trans_marker30.py input output 24 2")
        sys.exit(1)

    input_dir = sys.argv[1]
    output_dir = sys.argv[2]
    
    # Parse parameters
    alpha_bits = 2
    color_depth = 4
    save_png = False
    
    # Parse remaining arguments
    parsed_color_depth = False
    for arg in sys.argv[3:]:
        if arg.lower() == 'png':
            save_png = True
        elif arg in ['4', '5', '8', '24'] and not parsed_color_depth:
            color_depth = int(arg)
            parsed_color_depth = True
        elif arg in ['2', '4', '8']:
            alpha_bits = int(arg)
        else:
            print(f"Error: Invalid argument '{arg}'.")
            print("Color depth must be 4, 5, 8, or 24. Alpha bits must be 2, 4, or 8.")
            sys.exit(1)

    if not os.path.isdir(input_dir):
        print(f"Error: {input_dir} is not a valid directory.")
        sys.exit(1)

    print(f"Processing 30fps version with {alpha_bits}-bit alpha channel and {color_depth}-bit color depth")
    print(f"Save PNG files: {'Yes' if save_png else 'No (only .h files)'}")
    
    process_all_subdirectories(input_dir, output_dir, alpha_bits=alpha_bits, color_depth=color_depth, save_png=save_png)