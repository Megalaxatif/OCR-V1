import os
import random
from PIL import Image, ImageDraw, ImageFont

IMAGE_SIZE = 28
IMAGES_PER_CLASS = 1000

TRAIN_DIR = "../database/train5"

MIN_FONT_SIZE = 16
MAX_FONT_SIZE = 24
MAX_X_OFFSET = 1
MAX_Y_OFFSET = 1


def get_random_font(size):
    fonts = [
        # DejaVu
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSerif-Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",

        # Liberation
        "/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/liberation2/LiberationSans-Bold.ttf",
        "/usr/share/fonts/truetype/liberation2/LiberationSerif-Regular.ttf",
        "/usr/share/fonts/truetype/liberation2/LiberationSerif-Bold.ttf",
        "/usr/share/fonts/truetype/liberation2/LiberationMono-Regular.ttf",
        "/usr/share/fonts/truetype/liberation2/LiberationMono-Bold.ttf",

        # Ubuntu
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-B.ttf",

        # Noto
        "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
        "/usr/share/fonts/truetype/noto/NotoSans-Bold.ttf",
        "/usr/share/fonts/truetype/noto/NotoSerif-Regular.ttf",
        "/usr/share/fonts/truetype/noto/NotoSerif-Bold.ttf",

        # FreeFont
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSansBold.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSerif.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSerifBold.ttf",
        "/usr/share/fonts/truetype/freefont/FreeMono.ttf",
        "/usr/share/fonts/truetype/freefont/FreeMonoBold.ttf",
    ]

    fonts = [f for f in fonts if os.path.exists(f)]

    if not fonts:
        raise RuntimeError("Aucune police trouvée.")

    return ImageFont.truetype(random.choice(fonts), size)


def random_weight():
    return random.choices([0, 1], weights=[9, 1])[0]


def draw_text_weight(draw, x, y, text, font, w):
    if w == 0:
        draw.text((x, y), text, fill=0, font=font)
        return

    for dx in range(-w, w + 1):
        for dy in range(-w, w + 1):
            if dx * dx + dy * dy <= w * w:
                draw.text(
                    (x + dx, y + dy),
                    text,
                    fill=0,
                    font=font
                )


def add_local_noise(img):
    pixels = img.load()
    w, h = img.size

    # Récupère les pixels appartenant au chiffre
    text_pixels = []

    for y in range(h):
        for x in range(w):
            if pixels[x, y] < 220:
                text_pixels.append((x, y))

    if not text_pixels:
        return

    # Quelques petits amas de bruit autour du texte
    for _ in range(random.randint(2, 6)):
        cx, cy = random.choice(text_pixels)

        for _ in range(random.randint(1, 5)):
            x = cx + random.randint(-3, 3)
            y = cy + random.randint(-3, 3)

            if 0 <= x < w and 0 <= y < h:
                if random.random() < 0.3:
                    pixels[x, y] = random.randint(0, 80)
                elif random.random() < 0.3:
                    pixels[x, y] = random.randint(180, 255)


def generate_digit(digit):
    """
    Génère une image 28x28 d'un chiffre.

    Retourne :
        image : Image PIL
        garbage : True si le chiffre a été tourné
    """

    img = Image.new("L", (IMAGE_SIZE, IMAGE_SIZE), 255)
    draw = ImageDraw.Draw(img)

    # Même logique que l'ancien script :
    # un chiffre normal est généré sans rotation.
    # Ici, pour chaque image, on choisit aléatoirement
    # si elle doit être normale ou poubelle.
    if digit == "garbage":
        shown = str(random.randint(1, 9))
        angle = random.choice([90, 180, 270])
        garbage = True
    else:
        shown = str(digit)
        angle = 0
        garbage = False

    font = get_random_font(
        random.randint(MIN_FONT_SIZE, MAX_FONT_SIZE)
    )

    if angle == 0:
        bbox = draw.textbbox(
            (0, 0),
            shown,
            font=font
        )

        x = (
            (IMAGE_SIZE - (bbox[2] - bbox[0])) / 2
            + random.randint(-MAX_X_OFFSET, MAX_X_OFFSET)
        )

        y = (
            (IMAGE_SIZE - (bbox[3] - bbox[1])) / 2
            - bbox[1]
            + random.randint(-MAX_Y_OFFSET, MAX_Y_OFFSET)
        )

        draw_text_weight(
            draw,
            x,
            y,
            shown,
            font,
            random_weight()
        )

    else:
        # Même système que dans le script original
        tmp = Image.new("L", (64, 64), 255)
        td = ImageDraw.Draw(tmp)

        bbox = td.textbbox(
            (0, 0),
            shown,
            font=font
        )

        x = (64 - (bbox[2] - bbox[0])) / 2
        y = (
            (64 - (bbox[3] - bbox[1])) / 2
            - bbox[1]
        )

        draw_text_weight(
            td,
            x,
            y,
            shown,
            font,
            random_weight()
        )

        tmp = tmp.rotate(
            angle,
            expand=True,
            fillcolor=255
        )

        l = (tmp.width - IMAGE_SIZE) // 2
        t = (tmp.height - IMAGE_SIZE) // 2

        crop = tmp.crop(
            (
                l,
                t,
                l + IMAGE_SIZE,
                t + IMAGE_SIZE
            )
        )

        img.paste(crop, (0, 0))

    # Même augmentation locale
    if random.random() < 0.3:
        add_local_noise(img)

    # Même dégradation par downscale/upscale
    if random.random() < 0.3:
        scale = random.uniform(0.4, 0.9)

        down = random.choice([
            Image.Resampling.BILINEAR,
            Image.Resampling.BICUBIC,
            Image.Resampling.LANCZOS,
        ])

        up = random.choice([
            Image.Resampling.NEAREST,
            Image.Resampling.BILINEAR,
            Image.Resampling.BICUBIC,
        ])

        small = img.resize(
            (
                max(1, int(IMAGE_SIZE * scale)),
                max(1, int(IMAGE_SIZE * scale))
            ),
            down
        )

        img = small.resize(
            (IMAGE_SIZE, IMAGE_SIZE),
            up
        )

    return img, garbage


def generate_dataset():
    # Création des 10 classes normales
    for digit in range(10):
        os.makedirs(
            os.path.join(TRAIN_DIR, str(digit)),
            exist_ok=True
        )

    # Onzième classe
    os.makedirs(
        os.path.join(TRAIN_DIR, "garbage"),
        exist_ok=True
    )

    total = 11 * IMAGES_PER_CLASS
    current = 0

    # Chiffres 0-9
    for digit in range(10):
        output_dir = os.path.join(
            TRAIN_DIR,
            str(digit)
        )

        for i in range(IMAGES_PER_CLASS):
            img, _ = generate_digit(digit)

            img.save(
                os.path.join(
                    output_dir,
                    f"{i:04d}.png"
                )
            )

            current += 1
            print(f"{current}/{total}", end="\r")

    # Garbage
    output_dir = os.path.join(
        TRAIN_DIR,
        "garbage"
    )

    for i in range(IMAGES_PER_CLASS):
        img, _ = generate_digit("garbage")

        img.save(
            os.path.join(
                output_dir,
                f"{i:04d}.png"
            )
        )

        current += 1
        print(f"{current}/{total}", end="\r")

    print()
    print("Dataset généré.")
    print(f"Images : {total}")
    print(f"Taille : {IMAGE_SIZE}x{IMAGE_SIZE}")
    print(f"Dossier : {TRAIN_DIR}/")


if __name__ == "__main__":
    generate_dataset()
