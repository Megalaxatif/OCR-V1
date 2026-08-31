from pathlib import Path
import random
from PIL import Image, ImageDraw, ImageFont, ImageFilter


def generate_ocr_dataset(
    images_per_digit: int,
    output_dir: str = "../train4",
    seed: int | None = None
):
    if seed is not None:
        random.seed(seed)

    output = Path(output_dir)

    # Available fonts
    font_paths = [
        # DejaVu
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Oblique.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-BoldOblique.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSerif-Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSerif-Italic.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSerif-BoldItalic.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf",

        # Liberation
        "/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/liberation2/LiberationSans-Bold.ttf",
        "/usr/share/fonts/truetype/liberation2/LiberationSans-Italic.ttf",
        "/usr/share/fonts/truetype/liberation2/LiberationSans-BoldItalic.ttf",
        "/usr/share/fonts/truetype/liberation2/LiberationSerif-Regular.ttf",
        "/usr/share/fonts/truetype/liberation2/LiberationSerif-Bold.ttf",
        "/usr/share/fonts/truetype/liberation2/LiberationSerif-Italic.ttf",
        "/usr/share/fonts/truetype/liberation2/LiberationSerif-BoldItalic.ttf",
        "/usr/share/fonts/truetype/liberation2/LiberationMono-Regular.ttf",
        "/usr/share/fonts/truetype/liberation2/LiberationMono-Bold.ttf",

        # FreeFont
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSansBold.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSansOblique.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSansBoldOblique.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSerif.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSerifBold.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSerifItalic.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSerifBoldItalic.ttf",
        "/usr/share/fonts/truetype/freefont/FreeMono.ttf",
        "/usr/share/fonts/truetype/freefont/FreeMonoBold.ttf",

        # Noto
        "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
        "/usr/share/fonts/truetype/noto/NotoSans-Bold.ttf",
        "/usr/share/fonts/truetype/noto/NotoSerif-Regular.ttf",
        "/usr/share/fonts/truetype/noto/NotoSerif-Bold.ttf",

        # URW
        "/usr/share/fonts/type1/urw-base35/NimbusSans-Regular.t1",
        "/usr/share/fonts/type1/urw-base35/NimbusSans-Bold.t1",
        "/usr/share/fonts/type1/urw-base35/NimbusRoman-Regular.t1",
        "/usr/share/fonts/type1/urw-base35/NimbusRoman-Bold.t1",
    ]

    font_paths = [
        path for path in font_paths
        if Path(path).exists()
    ]

    if not font_paths:
        raise RuntimeError("No compatible font found.")

    for digit in range(10):
        digit_dir = output / str(digit)
        digit_dir.mkdir(parents=True, exist_ok=True)

        for i in range(images_per_digit):
            image = Image.new("L", (28, 28), 255)
            draw = ImageDraw.Draw(image)

            # Generate a large digit
            font = ImageFont.truetype(
                random.choice(font_paths),
                random.randint(24, 30)
            )

            bbox = draw.textbbox(
                (0, 0),
                str(digit),
                font=font
            )

            width = bbox[2] - bbox[0]
            height = bbox[3] - bbox[1]

            # Make sure the digit is large enough
            while width < 15 or height < 19:
                font = ImageFont.truetype(
                    random.choice(font_paths),
                    random.randint(27, 32)
                )

                bbox = draw.textbbox(
                    (0, 0),
                    str(digit),
                    font=font
                )

                width = bbox[2] - bbox[0]
                height = bbox[3] - bbox[1]

            # Center the digit
            x = (28 - width) // 2 - bbox[0]
            y = (28 - height) // 2 - bbox[1]

            draw.text(
                (x, y),
                str(digit),
                fill=0,
                font=font,
                stroke_width=random.choice([0, 0, 1, 1, 2]),
                stroke_fill=0
            )

            # Rotate the digit
            image = image.rotate(
                random.uniform(-15, 15),
                resample=Image.Resampling.BICUBIC,
                expand=False,
                fillcolor=255
            )

            # Slight translation
            translated = Image.new("L", (28, 28), 255)

            translated.paste(
                image,
                (
                    random.randint(-1, 1),
                    random.randint(-1, 1)
                )
            )

            image = translated

            # Slight blur
            if random.random() < 0.10:
                image = image.filter(
                    ImageFilter.GaussianBlur(
                        random.uniform(0.15, 0.4)
                    )
                )

            # Re-center the digit after transformations
            bbox = image.getbbox()

            if bbox is not None:
                left, top, right, bottom = bbox

                digit_width = right - left
                digit_height = bottom - top

                crop = image.crop(bbox)

                centered = Image.new(
                    "L",
                    (28, 28),
                    255
                )

                x = (28 - digit_width) // 2
                y = (28 - digit_height) // 2

                centered.paste(
                    crop,
                    (x, y)
                )

                image = centered

            # 30% of images contain no noise
            has_noise = random.random() >= 0.30

            if has_noise:
                draw = ImageDraw.Draw(image)

                # Random noise pixels
                for _ in range(random.randint(5, 35)):
                    x = random.randrange(28)
                    y = random.randrange(28)

                    image.putpixel(
                        (x, y),
                        0
                    )

                # Small noise clusters
                for _ in range(random.randint(0, 3)):
                    x = random.randrange(28)
                    y = random.randrange(28)

                    draw.point(
                        (x, y),
                        fill=0
                    )

                    if random.random() < 0.5:
                        draw.point(
                            (max(0, x - 1), y),
                            fill=0
                        )

                    if random.random() < 0.3:
                        draw.point(
                            (x, min(27, y + 1)),
                            fill=0
                        )

                # Light border noise
                if random.random() < 0.35:
                    for _ in range(random.randint(3, 15)):
                        side = random.randrange(4)

                        if side == 0:
                            x = random.randrange(28)
                            y = random.choice([0, 1])

                        elif side == 1:
                            x = random.randrange(28)
                            y = random.choice([26, 27])

                        elif side == 2:
                            x = random.choice([0, 1])
                            y = random.randrange(28)

                        else:
                            x = random.choice([26, 27])
                            y = random.randrange(28)

                        image.putpixel(
                            (x, y),
                            0
                        )

                # Rare partial border
                if random.random() < 0.08:
                    side = random.randrange(4)
                    length = random.randint(4, 10)

                    if side == 0:
                        x = random.randint(0, 28 - length)

                        draw.line(
                            [(x, 0), (x + length, 0)],
                            fill=0,
                            width=1
                        )

                    elif side == 1:
                        x = random.randint(0, 28 - length)

                        draw.line(
                            [(x, 27), (x + length, 27)],
                            fill=0,
                            width=1
                        )

                    elif side == 2:
                        y = random.randint(0, 28 - length)

                        draw.line(
                            [(0, y), (0, y + length)],
                            fill=0,
                            width=1
                        )

                    else:
                        y = random.randint(0, 28 - length)

                        draw.line(
                            [(27, y), (27, y + length)],
                            fill=0,
                            width=1
                        )

            # Save image
            image.save(
                digit_dir / f"{digit}_{i:05d}.png"
            )


generate_ocr_dataset(1000)
