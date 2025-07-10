import os
import sys
from PIL import Image, ImageColor

def parse_color(color_string, default_color=(255, 255, 255)):
    """
    Parse a color string to RGB tuple using PIL's ImageColor module.
    
    :param color_string: Color name or hex string
    :param default_color: Default color to use if parsing fails
    :return: RGB tuple
    """
    try:
        # Use PIL's ImageColor.getrgb() to parse the color
        return ImageColor.getrgb(color_string)
    except ValueError:
        # Fallback to default color if parsing fails
        print(f"Warning: Could not parse color '{color_string}', using default {default_color}")
        return default_color

def calculate_color_similarity(original_image, palette_image, threshold=0.95):
    """
    Calculate similarity between original and palette images using MSE.
    
    :param original_image: Original RGB image
    :param palette_image: Palette-based image converted back to RGB
    :param threshold: Similarity threshold (0.0 to 1.0)
    :return: (similarity_score, is_similar_enough)
    """
    # Convert images to RGB if needed
    orig_pixels = list(original_image.getdata())
    pal_pixels = list(palette_image.convert('RGB').getdata())
    
    # Calculate mean squared error
    total_squared_error = 0
    pixel_count = len(orig_pixels)
    
    for i in range(pixel_count):
        orig_r, orig_g, orig_b = orig_pixels[i]
        pal_r, pal_g, pal_b = pal_pixels[i]
        
        # Calculate squared differences for each channel
        r_diff = (orig_r - pal_r) ** 2
        g_diff = (orig_g - pal_g) ** 2
        b_diff = (orig_b - pal_b) ** 2
        
        total_squared_error += r_diff + g_diff + b_diff
    
    # Calculate MSE
    mse = total_squared_error / (pixel_count * 3)  # 3 channels
    
    # Convert MSE to similarity score (0-1, where 1 is identical)
    max_possible_mse = 255 ** 2
    similarity = 1 - (mse / max_possible_mse)
    
    return similarity, similarity >= threshold

def suggest_color_depth(original_image, palette_16_image, palette_256_image, threshold=0.98):
    """
    Suggest optimal color depth based on visual similarity.
    
    :param original_image: Original RGB image
    :param palette_16_image: 16-color palette image
    :param palette_256_image: 256-color palette image
    :param threshold_16: Similarity threshold for accepting 16-color (stricter)
    :param threshold_256: Similarity threshold for accepting 256-color
    :return: Suggested color depth (4 or 8)
    """
    # Calculate similarities
    sim_16, good_16 = calculate_color_similarity(original_image, palette_16_image, threshold)
    sim_256, good_256 = calculate_color_similarity(original_image, palette_256_image, threshold)
    
    print(f"    16-color similarity: {sim_16:.3f}, 256-color similarity: {sim_256:.3f}")
    
    # 更保守的判断：只有相似度很高时才选择16色
    if sim_16 >= 0.99:  # 非常严格的16色标准
        return 4, f"4 (16-color, similarity: {sim_16:.3f})"
    else:
        return 8, f"8 (256-color, similarity: {sim_256:.3f})"

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

    :param image: The image to convert.
    :param name: The base name for the C arrays.
    :param palette_size: The size of the palette (256 or 16).
    :return: A string containing the C array representation.
    """
    # Extract palette and pixel data
    palette = image.getpalette()[:palette_size * 3]  # Get the first `palette_size` colors
    pixels = list(image.getdata())

    # Convert palette to 0x00RRGGBB format
    palette_array = [
        f"0x{(palette[i] << 16 | palette[i + 1] << 8 | palette[i + 2]):06x}"
        for i in range(0, len(palette), 3)
    ]

    # Convert pixels to hex format
    if palette_size == 16:
        # Pack two 4-bit pixels into one byte
        packed_pixels = []
        for i in range(0, len(pixels), 2):
            pixel1 = pixels[i] & 0x0F  # First pixel (high 4 bits)
            pixel2 = pixels[i + 1] & 0x0F if i + 1 < len(pixels) else 0  # Second pixel (low 4 bits)
            packed_byte = (pixel1 << 4) | pixel2
            packed_pixels.append(f"0x{packed_byte:02x}")
        pixel_array = packed_pixels
    else:
        # 256-color palette uses one byte per pixel
        pixel_array = [f"0x{pixel:02x}" for pixel in pixels]

    # Format arrays with line breaks
    palette_c_array = f"const uint32_t {name}_palette_{palette_size}[] = {{\n    {format_c_array(palette_array)}\n}};"
    pixel_c_array = f"const uint8_t {name}_pixels_{palette_size}[] = {{\n    {format_c_array(pixel_array)}\n}};"

    return palette_c_array, pixel_c_array

def save_as_c_header(output_dir, name, all_images):
    """
    Save all image data as a C header file.

    :param output_dir: The directory to save the header file.
    :param name: The base name for the C arrays.
    :param all_images: Dictionary containing all processed images
    """
    header_file = os.path.join(output_dir, f"{name}.h")

    with open(header_file, "w") as f:
        f.write(f"#ifndef {name.upper()}_H\n")
        f.write(f"#define {name.upper()}_H\n\n")
        f.write(f"#include <stdint.h>\n\n")

        # Store suggestions for each image type
        suggestions = {}
        
        # First pass: analyze all images to get suggestions
        for image_type, image_data in all_images.items():
            output_image = image_data['output_image']
            palette_256_image = image_data['palette_256_image']
            palette_16_image = image_data['palette_16_image']
            
            # Get color depth suggestion
            suggested_depth, depth_reason = suggest_color_depth(output_image, palette_16_image, palette_256_image)
            suggestions[image_type] = suggested_depth

        # Calculate frame numbers for each animation type
        frame_nums = {}
        for image_type in all_images.keys():
            if image_type == 'miss':
                frame_nums[image_type] = 4 * 4  # 16 frames for miss
            else:
                frame_nums[image_type] = 4 * 8  # 32 frames for others

        # Generate DEF macro at the top
        f.write(f"/* Marker definition macro */\n")
        f.write(f"#define DEF_{name} {{ \\\n")
        
        # Generate each animation field
        animation_types = ['approach', 'perfect', 'great', 'good', 'poor', 'miss']
        for i, anim_type in enumerate(animation_types):
            if anim_type in suggestions:
                color_depth = suggestions[anim_type]
                frame_num = frame_nums[anim_type]
                
                if color_depth == 4:
                    palette_ptr = f"{name}_{anim_type}_palette_16"
                    img_ptr = f"{name}_{anim_type}_pixels_16"
                else:  # color_depth == 8
                    palette_ptr = f"{name}_{anim_type}_palette_256"
                    img_ptr = f"{name}_{anim_type}_pixels_256"
                
                f.write(f"    {{ {color_depth}, 13, {frame_num}, {palette_ptr}, .img8 = {img_ptr} }}")
            else:
                # Default empty animation if not found
                f.write(f"    {{ 0, 0, 0, NULL, .img8 = NULL }}")
            
            # 每一行都需要反斜杠，除了最后一行
            if i < len(animation_types) - 1:
                f.write(f", \\\n")
            else:
                f.write(f" \\\n")  # 最后一行也需要反斜杠
        
        f.write(f"}}\n\n")

        # Write only the selected image data arrays
        for image_type, image_data in all_images.items():
            output_image = image_data['output_image']
            palette_256_image = image_data['palette_256_image']
            palette_16_image = image_data['palette_16_image']
            
            suggested_depth = suggestions[image_type]
            
            # Write color depth suggestion comment
            f.write(f"/* {image_type.upper()} - Using {suggested_depth}-bit color depth */\n")

            if suggested_depth == 4:
                # Only write 16-color arrays
                palette_16_c_array, pixels_16_c_array = image_to_c_array(palette_16_image, f"{name}_{image_type}", 16)
                f.write(f"{palette_16_c_array}\n\n")
                f.write(f"{pixels_16_c_array}\n\n")
            else:  # suggested_depth == 8
                # Only write 256-color arrays
                palette_256_c_array, pixels_256_c_array = image_to_c_array(palette_256_image, f"{name}_{image_type}", 256)
                f.write(f"{palette_256_c_array}\n\n")
                f.write(f"{pixels_256_c_array}\n\n")
        
        f.write(f"#endif // {name.upper()}_H\n")

    print(f"Header file saved to {header_file}")

def process_image(input_file, output_file, grid_size=(4, 8), tile_size=(256, 256), target_size=(13, 13), background_color=(255, 255, 255), save_png=False):
    """
    Process an image by splitting it into a grid of sub-images, resizing each sub-image,
    and reassembling them into a new image in a vertical layout.

    :param input_file: Path to the input image file.
    :param output_file: Path to save the output image.
    :param grid_size: Tuple specifying the grid dimensions (columns, rows).
    :param tile_size: Tuple specifying the size of each tile in the grid (width, height).
    :param target_size: Tuple specifying the target size for each resized tile (width, height).
    :param background_color: Background color to use when removing transparency.
    :param save_png: Whether to save PNG files.
    :return: Dictionary containing processed images
    """
    with Image.open(input_file) as img:
        expected_size = (grid_size[0] * tile_size[0], grid_size[1] * tile_size[1])
        if img.size != expected_size:
            raise ValueError(f"Input image size must be {expected_size}, but got {img.size}")

        output_width = target_size[0]
        output_height = target_size[1] * (grid_size[0] * grid_size[1])
        output_image = Image.new("RGB", (output_width, output_height), background_color)

        tile_index = 0
        for row in range(grid_size[1]):
            for col in range(grid_size[0]):
                left = col * tile_size[0]
                upper = row * tile_size[1]
                right = left + tile_size[0]
                lower = upper + tile_size[1]
                tile_box = (left, upper, right, lower)

                tile = img.crop(tile_box)

                if tile.mode == "RGBA":
                    background = Image.new("RGBA", tile.size, background_color + (255,))
                    tile = Image.alpha_composite(background, tile).convert("RGB")

                tile = tile.resize(target_size, Image.LANCZOS)

                output_upper = tile_index * target_size[1]
                output_image.paste(tile, (0, output_upper))
                tile_index += 1

        # Save PNG files only if requested
        if save_png:
            # Save the original processed image
            output_image.save(output_file)
            print(f"Output saved to {output_file}")

            # Save the 256-color palette version
            palette_256_output_file = output_file.replace(".png", "_256.png")
            palette_256_image = output_image.convert("P", palette=Image.ADAPTIVE, colors=256)
            palette_256_image.save(palette_256_output_file)
            print(f"256-color palette version saved to {palette_256_output_file}")

            # Save the 16-color palette version
            palette_16_output_file = output_file.replace(".png", "_16.png")
            palette_16_image = output_image.convert("P", palette=Image.ADAPTIVE, colors=16)
            palette_16_image.save(palette_16_output_file)
            print(f"16-color palette version saved to {palette_16_output_file}")
        else:
            # Still need to create palette images for analysis, but don't save them
            palette_256_image = output_image.convert("P", palette=Image.ADAPTIVE, colors=256)
            palette_16_image = output_image.convert("P", palette=Image.ADAPTIVE, colors=16)

        return {
            'output_image': output_image,
            'palette_256_image': palette_256_image,
            'palette_16_image': palette_16_image
        }

def process_all_subdirectories(input_dir, output_dir, grid_size=(4, 8), tile_size=(256, 256), target_size=(13, 13), background_color=(255, 255, 255), save_png=False):
    """
    Process all subdirectories in the specified input directory, handling each image file,
    and save the output files in the specified output directory.

    :param input_dir: The base directory to search for subdirectories.
    :param output_dir: The directory to save the output files.
    :param grid_size: Tuple specifying the grid dimensions (columns, rows).
    :param tile_size: Tuple specifying the size of each tile in the grid (width, height).
    :param target_size: Tuple specifying the target size for each resized tile (width, height).
    :param background_color: Background color to use when removing transparency.
    :param save_png: Whether to save PNG files.
    """
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)  # Create the output directory if it doesn't exist

    # Define image types and their grid sizes
    image_configs = {
        'approach': {'grid_size': (4, 8)},
        'perfect': {'grid_size': (4, 8)},
        'great': {'grid_size': (4, 8)},
        'good': {'grid_size': (4, 8)},
        'poor': {'grid_size': (4, 8)},
        'miss': {'grid_size': (4, 4)}
    }

    for subdir in os.listdir(input_dir):
        subdir_path = os.path.join(input_dir, subdir)
        if os.path.isdir(subdir_path):  # Check if it's a directory
            all_images = {}
            
            print(f"Processing directory: {subdir}")
            
            for image_type, config in image_configs.items():
                input_file = os.path.join(subdir_path, f"{image_type}.png")
                if os.path.isfile(input_file):  # Check if the image file exists
                    print(f"  Processing {image_type}.png...")
                    output_file = os.path.join(output_dir, f"{subdir}_{image_type}.png")
                    try:
                        image_data = process_image(
                            input_file, 
                            output_file, 
                            config['grid_size'], 
                            tile_size, 
                            target_size, 
                            background_color,
                            save_png
                        )
                        all_images[image_type] = image_data
                    except Exception as e:
                        print(f"Failed to process {input_file}: {e}")

            # Generate C header file if any images were processed
            if all_images:
                print(f"  Analyzing color depth requirements...")
                save_as_c_header(output_dir, subdir, all_images)

if __name__ == "__main__":
    if len(sys.argv) < 3 or len(sys.argv) > 5:
        print("Usage: python3 approach.py <input_directory> <output_directory> [background_color] [png]")
        print("Background color can be a color name (white, black, red, etc.) or hex code (#RRGGBB)")
        print("Add 'png' at the end to save PNG files (default: only generate .h files)")
        sys.exit(1)

    input_dir = sys.argv[1]  # Input directory specified via command-line argument
    output_dir = sys.argv[2]  # Output directory specified via command-line argument
    
    # Parse background color if provided, otherwise use default white
    default_bg_color = (255, 255, 255)  # Default white background
    background_color = default_bg_color
    save_png = False
    
    # Parse optional arguments
    for i in range(3, len(sys.argv)):
        arg = sys.argv[i]
        if arg.lower() == 'png':
            save_png = True
        else:
            # Assume it's a background color
            background_color = parse_color(arg, default_bg_color)

    if not os.path.isdir(input_dir):
        print(f"Error: {input_dir} is not a valid directory.")
        sys.exit(1)

    print(f"Using background color: {background_color}")
    print(f"Save PNG files: {'Yes' if save_png else 'No (only .h files)'}")
    process_all_subdirectories(input_dir, output_dir, background_color=background_color, save_png=save_png)