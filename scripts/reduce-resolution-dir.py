import cv2
from pathlib import Path
import os

def resize_heightmap_tiles(input_dir, output_dir, target_size=256):
    in_path = Path(input_dir)
    out_path = Path(output_dir)

    if not in_path.exists():
        print(f"Error: Input directory {input_dir} not found.")
        return

    out_path.mkdir(parents=True, exist_ok=True)

    image_extensions = {'.png', '.tif', '.tiff'}
    files_to_process = [f for f in in_path.rglob('*') if f.suffix.lower() in image_extensions]
    total_files = len(files_to_process)

    print(f"Found {total_files} tiles to resize. Starting...")

    for i, img_file in enumerate(files_to_process, 1):
        img = cv2.imread(str(img_file), cv2.IMREAD_UNCHANGED)

        if img is None:
            print(f"Warning: Could not read {img_file}. Skipping.")
            continue

        resized = cv2.resize(img, (target_size, target_size), interpolation=cv2.INTER_AREA)

        rel_path = img_file.relative_to(in_path)
        out_file = out_path / rel_path

        out_file.parent.mkdir(parents=True, exist_ok=True)

        cv2.imwrite(str(out_file), resized)

        if i % 50 == 0 or i == total_files:
            print(f"Progress: {i}/{total_files} tiles processed.")

    print(f"Done! Resized tiles saved to: {out_path.absolute()}")

if __name__ == "__main__":
    INPUT_DIRECTORY = "/Users/ziv.perry/code/ziv/airborne/assets/tiles/north/z12-hmp"
    OUTPUT_DIRECTORY = "/Users/ziv.perry/code/ziv/airborne/assets/tiles/north/z12-hmp-521"

    resize_heightmap_tiles(INPUT_DIRECTORY, OUTPUT_DIRECTORY)