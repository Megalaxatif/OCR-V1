import os
import random
from PIL import Image, ImageDraw, ImageFont

# =========================
# Configuration
# =========================

OUTPUT_DIR = "../train3"
IMAGES_PER_DIGIT = 1000

IMAGE_SIZE = 28

# Taille du chiffre
FONT_SIZE_MIN = 21
FONT_SIZE_MAX = 23

# Rotation maximale
MAX_ROTATION = 3

# Petite variation de position
MAX_POSITION_OFFSET = 1

FONT_PATHS = [
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
]


# =========================
# Récupération des polices
# =========================

def get_available_fonts():
    fonts = []

    for path in FONT_PATHS:
        if os.path.isfile(path):
            fonts.append(path)

    if not fonts:
        raise RuntimeError(
            "Aucune police trouvée."
        )

    return fonts


# =========================
# Génération d'un chiffre
# =========================

def generate_digit(digit, fonts):

    # Image blanche 28x28
    image = Image.new(
        "L",
        (IMAGE_SIZE, IMAGE_SIZE),
        255
    )

    draw = ImageDraw.Draw(image)

    # Petite variation de taille
    font_size = random.randint(
        FONT_SIZE_MIN,
        FONT_SIZE_MAX
    )

    font_path = random.choice(fonts)

    font = ImageFont.truetype(
        font_path,
        font_size
    )

    # Calcul de la taille du chiffre
    bbox = draw.textbbox(
        (0, 0),
        str(digit),
        font=font
    )

    width = bbox[2] - bbox[0]
    height = bbox[3] - bbox[1]

    # Centrage
    x = (IMAGE_SIZE - width) // 2 - bbox[0]
    y = (IMAGE_SIZE - height) // 2 - bbox[1]

    # Très légère variation de position
    x += random.randint(
        -MAX_POSITION_OFFSET,
        MAX_POSITION_OFFSET
    )

    y += random.randint(
        -MAX_POSITION_OFFSET,
        MAX_POSITION_OFFSET
    )

    # Dessin du chiffre en noir
    draw.text(
        (x, y),
        str(digit),
        font=font,
        fill=0
    )

    # Rotation très légère
    angle = random.uniform(
        -MAX_ROTATION,
        MAX_ROTATION
    )

    image = image.rotate(
        angle,
        resample=Image.Resampling.BILINEAR,
        fillcolor=255
    )

    return image


# =========================
# Génération du dataset
# =========================

def create_dataset():

    fonts = get_available_fonts()

    os.makedirs(
        OUTPUT_DIR,
        exist_ok=True
    )

    total = 0

    for digit in range(10):

        digit_directory = os.path.join(
            OUTPUT_DIR,
            str(digit)
        )

        os.makedirs(
            digit_directory,
            exist_ok=True
        )

        print(f"Génération du chiffre {digit}...")

        for i in range(IMAGES_PER_DIGIT):

            image = generate_digit(
                digit,
                fonts
            )

            filename = f"{i:04d}.png"

            path = os.path.join(
                digit_directory,
                filename
            )

            image.save(path)

            total += 1

        print(
            f"  {IMAGES_PER_DIGIT} images générées"
        )

    print()
    print("Terminé.")
    print(f"Total : {total} images")
    print(f"Taille : {IMAGE_SIZE}x{IMAGE_SIZE}")
    print(f"Dossier : {OUTPUT_DIR}")


if __name__ == "__main__":
    create_dataset()
