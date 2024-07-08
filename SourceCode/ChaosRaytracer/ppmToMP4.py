from PIL import Image
import glob
import os
import cv2
import shutil

video_file = 'output.mp4'
input_path = './output/*.ppm'

images = sorted(glob.glob(input_path))

if not images:
    print("No images found in the directory.")
    exit()

frame = cv2.imread(images[0])
height, width, layers = frame.shape

fourcc = cv2.VideoWriter_fourcc(*'mp4v')  # Codec for MP4
video = cv2.VideoWriter(video_file, fourcc, 30, (width, height))

for image in images:
    img = cv2.imread(image)
    video.write(img)
    print(f"Added {image} to video.")

video.release()
