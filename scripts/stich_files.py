import os
from PIL import Image

ZOOM_LEVEL_DIR = r"/Users/ziv.perry/Desktop/tmpp/19"

OUTPUT_FILENAME = "stitched_map.png"
TILE_SIZE = 2048

def stitch_qgis_tiles(base_dir, output_filename):
    print(f"Scanning directory: {base_dir}...")

    x_folders = [f for f in os.listdir(base_dir) if os.path.isdir(os.path.join(base_dir, f)) and f.isdigit()]

    if not x_folders:
        print("Error: No X folders found. Make sure you point to a specific zoom level directory (e.g., .../12).")
        return

    x_coords = sorted([int(x) for x in x_folders])
    min_x, max_x = min(x_coords), max(x_coords)

    y_coords = []
    for x in x_coords:
        x_dir = os.path.join(base_dir, str(x))
        files = [f for f in os.listdir(x_dir) if f.endswith(('.png', '.jpg'))]
        for f in files:
            y_coords.append(int(f.split('.')[0]))

    if not y_coords:
        print("Error: No tile images found inside the X folders.")
        return

    min_y, max_y = min(y_coords), max(y_coords)

    width = (max_x - min_x + 1) * TILE_SIZE
    height = (max_y - min_y + 1) * TILE_SIZE

    print(f"Grid bounds: X({min_x} to {max_x}), Y({min_y} to {max_y})")
    print(f"Creating output image: {width} x {height} pixels...")

    Image.MAX_IMAGE_PIXELS = None

    stitched_image = Image.new('RGBA', (width, height), (0, 0, 0, 0))

    tiles_processed = 0
    total_tiles = (max_x - min_x + 1) * (max_y - min_y + 1)

    for x in range(min_x, max_x + 1):
        for y in range(min_y, max_y + 1):
            # חיפוש PNG או JPG
            tile_path_png = os.path.join(base_dir, str(x), f"{y}.png")
            tile_path_jpg = os.path.join(base_dir, str(x), f"{y}.jpg")

            tile_path = tile_path_png if os.path.exists(tile_path_png) else tile_path_jpg

            if os.path.exists(tile_path):
                try:
                    with Image.open(tile_path) as tile:
                        # חישוב פיקסלים מדויק על הקנבס
                        paste_x = (x - min_x) * TILE_SIZE
                        paste_y = (y - min_y) * TILE_SIZE

                        stitched_image.paste(tile, (paste_x, paste_y))
                        tiles_processed += 1
                except Exception as e:
                    print(f"Error pasting tile {x}/{y}: {e}")

    print(f"Processed {tiles_processed} tiles.")
    print("Saving file (this might take a while for huge maps)...")

    stitched_image.save(output_filename)
    print(f"Done! Saved to {output_filename}")

if __name__ == "__main__":
    stitch_qgis_tiles(ZOOM_LEVEL_DIR, OUTPUT_FILENAME)