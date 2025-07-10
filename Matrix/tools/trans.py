from PIL import Image  # 确保导入 Resampling
import os
import re

def resize_and_concatenate_images(input_folder, output_file, target_size=(13, 13), background_color=(0, 0, 0)):
    """
    Resize PNG images to a target size and concatenate them horizontally.

    :param input_folder: Folder containing the input PNG files.
    :param output_file: Path to save the concatenated output image.
    :param target_size: Tuple specifying the target resolution (width, height).
    :param background_color: Background color to use when removing transparency.
    """
    # Get a sorted list of PNG files matching the pattern hXXX.png
    files = sorted(
        [f for f in os.listdir(input_folder) if re.match(r'ma\d{2}\.png', f)],
        key=lambda x: int(re.search(r'\d+', x).group())
    )

    resized_images = []
    for file in files:
        file_path = os.path.join(input_folder, file)
        with Image.open(file_path) as img:
            # Remove alpha channel by compositing on a black background
            img = img.convert("RGBA")
            background = Image.new("RGBA", img.size, background_color)  # Ensure background is RGBA
            img = Image.alpha_composite(background, img).convert("RGB")
            
            # Resize to target size
            img = img.resize(target_size, Image.LANCZOS)
            resized_images.append(img)

    # Concatenate images horizontally
    if resized_images:
        total_width = target_size[0] * len(resized_images)
        concatenated_image = Image.new("RGB", (total_width, target_size[1]), background_color)

        x_offset = 0
        for img in resized_images:
            concatenated_image.paste(img, (x_offset, 0))
            x_offset += target_size[0]

        # Save the concatenated image
        concatenated_image.save(output_file)
        print(f"Output saved to {output_file}")
    else:
        print("No valid PNG files found in the input folder.")

if __name__ == "__main__":
    input_folder = "."  # Current directory
    output_file = "output.png"
    resize_and_concatenate_images(input_folder, output_file, background_color="white")