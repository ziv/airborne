import os
import shutil
from pathlib import Path

def normalize_tiles_to_grid(input_dir, output_dir):
    in_path = Path(input_dir)
    out_path = Path(output_dir)

    if not in_path.exists():
        print(f"Error: Input directory {input_dir} not found.")
        return

    out_path.mkdir(parents=True, exist_ok=True)

    tiles = []

    print("Scanning directory for tiles...")
    for file_path in in_path.rglob('*'):
        if file_path.suffix.lower() in ['.png', '.jpg', '.tif', '.tiff']:
            try:
                orig_x = int(file_path.parent.name)
                orig_y = int(file_path.stem)
                tiles.append((file_path, orig_x, orig_y))
            except ValueError:
                continue

    if not tiles:
        print("No tiles found in the specified directory.")
        return

    min_x = min(t[1] for t in tiles)
    min_y = min(t[2] for t in tiles)

    print(f"Found {len(tiles)} tiles.")
    print(f"Top-Left origin point is: X={min_x}, Y={min_y}")
    print("Copying and renaming files...")

    for i, (file_path, orig_x, orig_y) in enumerate(tiles, 1):
        grid_x = orig_x - min_x
        grid_z = orig_y - min_y

        new_filename = f"tex-{grid_x}-{grid_z}{file_path.suffix.lower()}"
        new_filepath = out_path / new_filename

        shutil.copy2(file_path, new_filepath)

        if i % 100 == 0 or i == len(tiles):
            print(f"Processed {i}/{len(tiles)} tiles...")

    print(f"\nSuccess! All tiles are normalized to a 0,0 grid.")
    print(f"Saved to: {out_path.absolute()}")

if __name__ == "__main__":
    INPUT_DIR = "/Users/ziv.perry/Desktop/tmpp/12-tex-greece"
    OUTPUT_DIR = "/Users/ziv.perry/code/ziv/airborne/assets/tiles-greece"

    normalize_tiles_to_grid(INPUT_DIR, OUTPUT_DIR)