import os
import math
import argparse
from PIL import Image


def split_image(image_path, output_dir, num_parts):
    # cancel size limit for large maps
    Image.MAX_IMAGE_PIXELS = None

    # input validation
    grid_size = math.sqrt(num_parts)
    if not grid_size.is_integer():
        print(f"Error: num_parts ({num_parts}) must be a perfect square (e.g., 4, 9, 16, 64).")
        return

    grid_size = int(grid_size)

    # target directory
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
        print(f"Created output directory: {output_dir}")

    # loading image
    try:
        img = Image.open(image_path)
    except Exception as e:
        print(f"Error opening image: {e}")
        return

    width, height = img.size
    print(f"Opened {image_path} (Size: {width}x{height}, Mode: {img.mode})")

    chunk_width = width // grid_size
    chunk_height = height // grid_size

    base_name = os.path.splitext(os.path.basename(image_path))[0]
    ext = os.path.splitext(image_path)[1]

    print(f"Splitting into {grid_size}x{grid_size} grid. Each chunk will be {chunk_width}x{chunk_height} pixels.")

    count = 0
    for y in range(grid_size):
        for x in range(grid_size):
            left = x * chunk_width
            upper = y * chunk_height
            right = left + chunk_width
            lower = upper + chunk_height

            chunk = img.crop((left, upper, right, lower))

            chunk_filename = f"{base_name}-{x}-{y}{ext}"
            chunk_filepath = os.path.join(output_dir, chunk_filename)

            chunk.save(chunk_filepath)
            count += 1
            print(f"Saved: {chunk_filename}")

    print(f"Success! Splitted image into {count} chunks in '{output_dir}'.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Split a large image into a grid of smaller images.")
    parser.add_argument("image_path", help="Path to the original image file")
    parser.add_argument("output_dir", help="Directory to save the split images")
    parser.add_argument("num_parts", type=int, help="Number of total parts (must be a perfect square)")

    args = parser.parse_args()

    split_image(args.image_path, args.output_dir, args.num_parts)
