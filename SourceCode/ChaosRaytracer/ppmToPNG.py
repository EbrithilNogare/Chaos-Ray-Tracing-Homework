import os
from PIL import Image
import numpy as np

source_folder = 'output'
destination_folder = '../../Images/13'

os.makedirs(destination_folder, exist_ok=True)

def ppm_p3_to_png(ppm_file_path, png_file_path):
    with open(ppm_file_path, 'r') as file:
        header = file.readline().strip()
        assert header == 'P3', "Invalid PPM header"

        dimensions = file.readline().strip().split()
        width = int(dimensions[0])
        height = int(dimensions[1])

        max_color = int(file.readline().strip())

        pixel_data = file.read().strip().split()
        
        pixel_values = [int(value) for value in pixel_data]

        expected_pixel_count = width * height * 3
        if len(pixel_values) != expected_pixel_count:
            raise ValueError(f"Expected {expected_pixel_count} pixel values, but found {len(pixel_values)}")

        pixel_array = np.array(pixel_values, dtype=np.uint8).reshape((height, width, 3))

        image = Image.fromarray(pixel_array, 'RGB')
        image.save(png_file_path)


for filename in os.listdir(source_folder):
    if filename.lower().endswith('.ppm'):
        source_path = os.path.join(source_folder, filename)
        destination_path = os.path.join(destination_folder, os.path.splitext(filename)[0] + '.png')
        
        ppm_p3_to_png(source_path, destination_path)

        print(f"Converted {filename} to {os.path.basename(destination_path)}")
