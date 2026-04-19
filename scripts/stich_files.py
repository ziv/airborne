import os
import re
from PIL import Image

Image.MAX_IMAGE_PIXELS = None

def stitch_tiles(directory, output_filename="stitched_map.png", tile_size=1024):
    tiles = []
    min_x, max_x = float('inf'), float('-inf')
    min_y, max_y = float('inf'), float('-inf')

    pattern = re.compile(r'(\d+)[_/\\-](\d+)\.(png|jpg|jpeg)$', re.IGNORECASE)

    for root, _, files in os.walk(directory):
        for file in files:
            filepath = os.path.join(root, file)
            normalized_path = filepath.replace('\\', '/')
            match = pattern.search(normalized_path)

            if match:
                x, y = int(match.group(1)), int(match.group(2))
                tiles.append((x, y, filepath))

                min_x = min(min_x, x)
                max_x = max(max_x, x)
                min_y = min(min_y, y)
                max_y = max(max_y, y)

    if not tiles:
        print("Error: No tiles found.")
        print("Expected file structure: .../x/y.png OR .../x_y.png")
        return

    print(f"Found {len(tiles)} tiles.")
    print(f"X range: {min_x} to {max_x}")
    print(f"Y range: {min_y} to {max_y}")

    grid_width = max_x - min_x + 1
    grid_height = max_y - min_y + 1

    total_width = grid_width * tile_size
    total_height = grid_height * tile_size

    print(f"Creating canvas of size {total_width}x{total_height} pixels...")

    merged_image = Image.new('RGBA', (total_width, total_height), (0, 0, 0, 0))

    for x, y, filepath in tiles:
        try:
            with Image.open(filepath) as tile:
                paste_x = (x - min_x) * tile_size
                paste_y = (y - min_y) * tile_size
                merged_image.paste(tile, (paste_x, paste_y))
        except Exception as e:
            print(f"Error processing {filepath}: {e}")

    print(f"Saving merged image to {output_filename}...")
    merged_image.save(output_filename)
    print("Done! Image successfully created.")

if __name__ == "__main__":
    TARGET_DIRECTORY = "/Users/ziv.perry/Desktop/tmpp/12"
    OUTPUT_FILE = "final_qgis_map.png"

    stitch_tiles(TARGET_DIRECTORY, OUTPUT_FILE, tile_size=1024)