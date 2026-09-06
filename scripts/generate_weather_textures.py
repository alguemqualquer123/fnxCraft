#!/usr/bin/env python3
from PIL import Image, ImageDraw
import os

OUT = "resources/assets/weather"
os.makedirs(OUT, exist_ok=True)

def save(img, name):
    img.save(f"{OUT}/{name}.png", "PNG")
    print(f"Generated {name}.png ({img.size})")

# Rain streak texture (diagonal rain drop)
def gen_rain_streak():
    im = Image.new("RGBA", (64, 64), (0,0,0,0))
    d = ImageDraw.Draw(im)
    d.line([(16, 0), (48, 64)], fill=(140, 170, 220, 255), width=3)
    d.line([(20, 0), (52, 64)], fill=(180, 200, 240, 255), width=2)
    save(im, "rain_streak")

# Rain sheet texture (soft rain area)
def gen_rain_sheet():
    im = Image.new("RGBA", (64, 64), (0,0,0,0))
    d = ImageDraw.Draw(im)
    for i in range(20):
        x = 10 + i * 2.5
        d.line([(x, 0), (x + 2, 64)], fill=(120, 150, 200, 40), width=1)
    save(im, "rain_sheet")

# Snowflake texture
def gen_snowflake():
    im = Image.new("RGBA", (32, 32), (0,0,0,0))
    d = ImageDraw.Draw(im)
    cx, cy = 16, 16
    for angle in range(0, 360, 30):
        rad = angle * 3.14159 / 180
        end = (cx + 12 * __import__('math').cos(rad), cy + 12 * __import__('math').sin(rad))
        d.line([(cx, cy), end], fill=(220, 230, 255, 255), width=2)
    d.ellipse([10, 10, 22, 22], outline=(240, 245, 255, 255), width=1)
    save(im, "snowflake")

# Snow sheet texture
def gen_snow_sheet():
    im = Image.new("RGBA", (64, 64), (0,0,0,0))
    d = ImageDraw.Draw(im)
    for i in range(15):
        x = 8 + i * 4
        y = 8 + i * 3
        d.ellipse([x, y, x+3, y+3], fill=(210, 220, 240, 80))
    save(im, "snow_sheet")

# Lightning bolt texture
def gen_lightning_bolt():
    im = Image.new("RGBA", (32, 64), (0,0,0,0))
    d = ImageDraw.Draw(im)
    points = [(16,0), (12,15), (20,15), (8,30), (24,30), (12,48), (16,64)]
    for i in range(len(points)-1):
        d.line([points[i], points[i+1]], fill=(255, 250, 180, 255), width=3)
    d.line([(14,0), (18,0)], fill=(255, 255, 200, 255), width=4)
    save(im, "lightning_bolt")

# Flash texture (white glow)
def gen_flash():
    im = Image.new("RGBA", (64, 64), (0,0,0,0))
    d = ImageDraw.Draw(im)
    d.ellipse([8, 8, 56, 56], fill=(255, 255, 240, 180))
    d.ellipse([16, 16, 48, 48], fill=(255, 255, 230, 120))
    save(im, "flash")

# Lens dirt texture
def gen_lens_dirt():
    im = Image.new("RGBA", (64, 64), (0,0,0,0))
    d = ImageDraw.Draw(im)
    d.ellipse([16, 16, 48, 48], fill=(120, 140, 160, 80))
    d.ellipse([20, 20, 44, 44], fill=(100, 120, 140, 40))
    save(im, "lens_dirt")

gen_rain_streak()
gen_rain_sheet()
gen_snowflake()
gen_snow_sheet()
gen_lightning_bolt()
gen_flash()
gen_lens_dirt()
print("\nDone! All weather textures generated.")
