"""Ghép các khối ảnh trong received/ thành file .jpg: python tools/assemble_image.py 57"""
import glob, sys
img = sys.argv[1]
parts = sorted(glob.glob(f"received/img_{img}.part*"))
with open(f"received/img_{img}.jpg", "wb") as out:
    for p in parts: out.write(open(p, "rb").read())
print("Ghép", len(parts), "khối ->", f"received/img_{img}.jpg")
