#include <SDL2/SDL.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define PI 3.14159265358979323846
#define MAX_SKEW_ANGLE 20.0
#define ANGLE_STEP 0.25
#define BLACK_THRESHOLD 128
#define HOUGH_PEAK_COUNT 10

static int IsBlackPixel(SDL_Surface *surface, int x, int y)
{
    Uint32 *pixels = (Uint32 *)surface->pixels;
    int pitch = surface->pitch / sizeof(Uint32);

    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;

    SDL_GetRGBA(
        pixels[y * pitch + x],
        surface->format,
        &r,
        &g,
        &b,
        &a
    );

    /*
     * The surface is already grayscale, so testing the red channel
     * is enough to know whether the pixel is dark.
     */
    return r < BLACK_THRESHOLD;
}

static int GetAccumulatorScore(const int *accumulator, int size)
{
    int best[HOUGH_PEAK_COUNT] = {0};

    /*
     * Keep the strongest Hough peaks.
     * This is useful for grid images because several parallel lines
     * should vote strongly for the same angle.
     */
    for (int i = 0; i < size; i++)
    {
        int value = accumulator[i];

        for (int j = 0; j < HOUGH_PEAK_COUNT; j++)
        {
            if (value > best[j])
            {
                for (int k = HOUGH_PEAK_COUNT - 1; k > j; k--)
                    best[k] = best[k - 1];

                best[j] = value;
                break;
            }
        }
    }

    int score = 0;

    for (int i = 0; i < HOUGH_PEAK_COUNT; i++)
        score += best[i];

    return score;
}

static double FindSkewAngle(SDL_Surface *surface)
{
    int width = surface->w;
    int height = surface->h;

    int diagonal = (int)ceil(
        sqrt(
            (double)width * width +
            (double)height * height
        )
    );

    int rhoCount = diagonal * 2 + 1;

    int *horizontalAccumulator =
        calloc((size_t)rhoCount, sizeof(int));

    int *verticalAccumulator =
        calloc((size_t)rhoCount, sizeof(int));

    if (horizontalAccumulator == NULL ||
        verticalAccumulator == NULL)
    {
        free(horizontalAccumulator);
        free(verticalAccumulator);

        SDL_SetError(
            "FindSkewAngle: failed to allocate Hough accumulators"
        );

        return 0.0;
    }

    double bestAngle = 0.0;
    int bestScore = -1;

    if (SDL_MUSTLOCK(surface))
    {
        if (SDL_LockSurface(surface) != 0)
        {
            free(horizontalAccumulator);
            free(verticalAccumulator);
            return 0.0;
        }
    }

    /*
     * Test every possible skew angle.
     */
    for (double angle = -MAX_SKEW_ANGLE;
         angle <= MAX_SKEW_ANGLE;
         angle += ANGLE_STEP)
    {
        for (int i = 0; i < rhoCount; i++)
        {
            horizontalAccumulator[i] = 0;
            verticalAccumulator[i] = 0;
        }

        /*
         * For horizontal lines, the normal is around 90 degrees.
         */
        double horizontalTheta =
            (90.0 + angle) * PI / 180.0;

        /*
         * For vertical lines, the normal is around 0 degrees.
         */
        double verticalTheta =
            angle * PI / 180.0;

        double horizontalCos = cos(horizontalTheta);
        double horizontalSin = sin(horizontalTheta);

        double verticalCos = cos(verticalTheta);
        double verticalSin = sin(verticalTheta);

        /*
         * Every black pixel votes in both horizontal and vertical
         * Hough accumulators.
         */
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                if (!IsBlackPixel(surface, x, y))
                    continue;

                double horizontalRho =
                    x * horizontalCos +
                    y * horizontalSin;

                double verticalRho =
                    x * verticalCos +
                    y * verticalSin;

                int horizontalIndex =
                    (int)round(horizontalRho) + diagonal;

                int verticalIndex =
                    (int)round(verticalRho) + diagonal;

                if (horizontalIndex >= 0 &&
                    horizontalIndex < rhoCount)
                {
                    horizontalAccumulator[horizontalIndex]++;
                }

                if (verticalIndex >= 0 &&
                    verticalIndex < rhoCount)
                {
                    verticalAccumulator[verticalIndex]++;
                }
            }
        }

        int horizontalScore =
            GetAccumulatorScore(
                horizontalAccumulator,
                rhoCount
            );

        int verticalScore =
            GetAccumulatorScore(
                verticalAccumulator,
                rhoCount
            );

        int score =
            horizontalScore +
            verticalScore;

        if (score > bestScore)
        {
            bestScore = score;
            bestAngle = angle;
        }
    }

    if (SDL_MUSTLOCK(surface))
        SDL_UnlockSurface(surface);

    free(horizontalAccumulator);
    free(verticalAccumulator);

    return bestAngle;
}

static SDL_Surface *RotateSurface(
    SDL_Surface *source,
    double angle)
{
    double rad = angle * PI / 180.0;

    double c = cos(rad);
    double s = sin(rad);

    int srcWidth = source->w;
    int srcHeight = source->h;

    /*
     * Compute a destination size large enough to contain
     * the entire rotated image.
     */
    int dstWidth = (int)ceil(
        fabs(srcWidth * c) +
        fabs(srcHeight * s)
    );

    int dstHeight = (int)ceil(
        fabs(srcWidth * s) +
        fabs(srcHeight * c)
    );

    SDL_Surface *destination =
        SDL_CreateRGBSurfaceWithFormat(
            0,
            dstWidth,
            dstHeight,
            32,
            SDL_PIXELFORMAT_RGBA32
        );

    if (destination == NULL)
        return NULL;

    /*
     * Fill empty areas with white.
     */
    Uint32 white = SDL_MapRGBA(
        destination->format,
        255,
        255,
        255,
        255
    );

    if (SDL_FillRect(destination, NULL, white) != 0)
    {
        SDL_FreeSurface(destination);
        return NULL;
    }

    double srcCX = (srcWidth - 1) / 2.0;
    double srcCY = (srcHeight - 1) / 2.0;

    double dstCX = (dstWidth - 1) / 2.0;
    double dstCY = (dstHeight - 1) / 2.0;

    if (SDL_MUSTLOCK(source))
    {
        if (SDL_LockSurface(source) != 0)
        {
            SDL_FreeSurface(destination);
            return NULL;
        }
    }

    if (SDL_MUSTLOCK(destination))
    {
        if (SDL_LockSurface(destination) != 0)
        {
            if (SDL_MUSTLOCK(source))
                SDL_UnlockSurface(source);

            SDL_FreeSurface(destination);
            return NULL;
        }
    }

    Uint32 *srcPixels = (Uint32 *)source->pixels;
    Uint32 *dstPixels = (Uint32 *)destination->pixels;

    int srcPitch =
        source->pitch / sizeof(Uint32);

    int dstPitch =
        destination->pitch / sizeof(Uint32);

    /*
     * Use inverse mapping:
     * for each destination pixel, compute the corresponding
     * position in the source image.
     */
    for (int y = 0; y < dstHeight; y++)
    {
        for (int x = 0; x < dstWidth; x++)
        {
            double dx = x - dstCX;
            double dy = y - dstCY;

            double srcX =
                c * dx +
                s * dy +
                srcCX;

            double srcY =
                -s * dx +
                c * dy +
                srcCY;

            int sx = (int)round(srcX);
            int sy = (int)round(srcY);

            if (sx < 0 ||
                sx >= srcWidth ||
                sy < 0 ||
                sy >= srcHeight)
            {
                continue;
            }

            dstPixels[y * dstPitch + x] =
                srcPixels[sy * srcPitch + sx];
        }
    }

    if (SDL_MUSTLOCK(destination))
        SDL_UnlockSurface(destination);

    if (SDL_MUSTLOCK(source))
        SDL_UnlockSurface(source);

    return destination;
}

SDL_Surface *DeskewSurface(SDL_Surface *grayscale)
{
    /*
     * Reject invalid input.
     */
    if (grayscale == NULL)
    {
        SDL_SetError(
            "DeskewSurface: input surface is NULL"
        );

        return NULL;
    }

    /*
     * The input surface must use RGBA32.
     */
    if (grayscale->format == NULL ||
        grayscale->format->format != SDL_PIXELFORMAT_RGBA32)
    {
        SDL_SetError(
            "DeskewSurface: input surface must use SDL_PIXELFORMAT_RGBA32"
        );

        return NULL;
    }

    /*
     * Find the skew angle using the Hough transform.
     */
    double angle =
        FindSkewAngle(grayscale);

    printf(
        "Detected skew angle: %.2f degrees\n",
        angle
    );

    /*
     * Rotate in the opposite direction to straighten the image.
     */
    SDL_Surface *result =
        RotateSurface(
            grayscale,
            -angle
        );

    return result;
}
