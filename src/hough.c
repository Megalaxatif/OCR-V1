#include <SDL2/SDL.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAX_SKEW_ANGLE 80
#define ANGLE_STEP 0.25

static void getRgbPixel(
    const SDL_Surface *surface,
    int x,
    int y,
    Uint8 *r,
    Uint8 *g,
    Uint8 *b,
    Uint8 *a)
{
    Uint32 *row = (Uint32 *)(
        (Uint8 *)surface->pixels + (size_t)y * surface->pitch);

    Uint32 pixel = row[x];

    SDL_GetRGBA(pixel, surface->format, r, g, b, a);
}

static void setRgbaPixel(
    SDL_Surface *surface,
    int x,
    int y,
    Uint8 r,
    Uint8 g,
    Uint8 b,
    Uint8 a)
{
    Uint32 *row = (Uint32 *)(
        (Uint8 *)surface->pixels + (size_t)y * surface->pitch);

    row[x] = SDL_MapRGBA(
        surface->format,
        r,
        g,
        b,
        a);
}

static Uint8 getBinaryValue(
    const SDL_Surface *surface,
    int x,
    int y)
{
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;

    getRgbPixel(
        surface,
        x,
        y,
        &r,
        &g,
        &b,
        &a);

    return r == 0 ? 0 : 1;
}

static Uint8 findForegroundValue(
    const SDL_Surface *surface)
{
    size_t blackCount = 0;
    size_t whiteCount = 0;

    for (int y = 0; y < surface->h; y++)
    {
        for (int x = 0; x < surface->w; x++)
        {
            if (getBinaryValue(surface, x, y) == 0)
                blackCount++;
            else
                whiteCount++;
        }
    }

    /*
     * On suppose que le fond occupe la majorité de l'image.
     */
    if (blackCount < whiteCount)
        return 0;

    return 1;
}

static double findSkewAngle(
    const SDL_Surface *surface,
    Uint8 foreground)
{
    int width = surface->w;
    int height = surface->h;

    int maxRho = (int)ceil(
        hypot(
            (double)width,
            (double)height));

    int rhoCount = 2 * maxRho + 1;

    int *accumulator = calloc(
        (size_t)rhoCount,
        sizeof(int));

    if (accumulator == NULL)
        return 0.0;

    double bestAngle = 0.0;
    uint64_t bestScore = 0;

    for (double angle = -MAX_SKEW_ANGLE;
         angle <= MAX_SKEW_ANGLE;
         angle += ANGLE_STEP)
    {
        memset(
            accumulator,
            0,
            (size_t)rhoCount * sizeof(int));

        double radians = angle * M_PI / 180.0;

        double cosAngle = cos(radians);
        double sinAngle = sin(radians);

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                if (getBinaryValue(surface, x, y) != foreground)
                    continue;

                double rho =
                    (double)x * cosAngle +
                    (double)y * sinAngle;

                int rhoIndex =
                    (int)lround(rho) + maxRho;

                if (rhoIndex >= 0 &&
                    rhoIndex < rhoCount)
                {
                    accumulator[rhoIndex]++;
                }
            }
        }

        uint64_t score = 0;

        for (int i = 0; i < rhoCount; i++)
        {
            uint64_t value =
                (uint64_t)accumulator[i];

            score += value * value;
        }

        if (score > bestScore)
        {
            bestScore = score;
            bestAngle = angle;
        }
    }

    free(accumulator);

    return bestAngle;
}

static SDL_Surface *rotateBinaryRgba32(
    const SDL_Surface *source,
    double angle,
    Uint8 background)
{
    double radians = angle * M_PI / 180.0;

    double cosAngle = cos(radians);
    double sinAngle = sin(radians);

    int sourceWidth = source->w;
    int sourceHeight = source->h;

    int destinationWidth = (int)ceil(
        fabs((double)sourceWidth * cosAngle) +
        fabs((double)sourceHeight * sinAngle));

    int destinationHeight = (int)ceil(
        fabs((double)sourceWidth * sinAngle) +
        fabs((double)sourceHeight * cosAngle));

    SDL_Surface *destination =
        SDL_CreateRGBSurfaceWithFormat(
            0,
            destinationWidth,
            destinationHeight,
            32,
            SDL_PIXELFORMAT_RGBA32);

    if (destination == NULL)
        return NULL;

    Uint8 backgroundValue =
        background == 0 ? 0 : 255;

    Uint32 backgroundPixel =
        SDL_MapRGBA(
            destination->format,
            backgroundValue,
            backgroundValue,
            backgroundValue,
            255);

    SDL_FillRect(
        destination,
        NULL,
        backgroundPixel);

    double sourceCenterX =
        ((double)sourceWidth - 1.0) / 2.0;

    double sourceCenterY =
        ((double)sourceHeight - 1.0) / 2.0;

    double destinationCenterX =
        ((double)destinationWidth - 1.0) / 2.0;

    double destinationCenterY =
        ((double)destinationHeight - 1.0) / 2.0;

    if (SDL_MUSTLOCK(destination))
    {
        if (SDL_LockSurface(destination) != 0)
        {
            SDL_FreeSurface(destination);
            return NULL;
        }
    }

    for (int y = 0; y < destinationHeight; y++)
    {
        for (int x = 0; x < destinationWidth; x++)
        {
            double deltaX =
                (double)x - destinationCenterX;

            double deltaY =
                (double)y - destinationCenterY;

            double sourceX =
                cosAngle * deltaX +
                sinAngle * deltaY +
                sourceCenterX;

            double sourceY =
                -sinAngle * deltaX +
                cosAngle * deltaY +
                sourceCenterY;

            int sourcePixelX =
                (int)lround(sourceX);

            int sourcePixelY =
                (int)lround(sourceY);

            if (sourcePixelX < 0 ||
                sourcePixelX >= sourceWidth ||
                sourcePixelY < 0 ||
                sourcePixelY >= sourceHeight)
            {
                continue;
            }

            Uint8 value = getBinaryValue(
                source,
                sourcePixelX,
                sourcePixelY);

            Uint8 rgb =
                value == 0 ? 0 : 255;

            setRgbaPixel(
                destination,
                x,
                y,
                rgb,
                rgb,
                rgb,
                255);
        }
    }

    if (SDL_MUSTLOCK(destination))
        SDL_UnlockSurface(destination);

    return destination;
}

SDL_Surface *DeskewSurface(SDL_Surface *surface)
{
    if (surface == NULL)
    {
        SDL_SetError("DeskewSurface: surface is NULL");
        return NULL;
    }

    if (surface->format->format != SDL_PIXELFORMAT_RGBA32)
    {
        SDL_SetError(
            "DeskewSurface: surface must be SDL_PIXELFORMAT_RGBA32");

        return NULL;
    }

    if (SDL_MUSTLOCK(surface))
    {
        if (SDL_LockSurface(surface) != 0)
            return NULL;
    }

    Uint8 foreground =
        findForegroundValue(surface);

    Uint8 background =
        foreground == 0 ? 1 : 0;

    double skewAngle =
        findSkewAngle(
            surface,
            foreground);

    SDL_Surface *result =
        rotateBinaryRgba32(
            surface,
            -skewAngle,
            background);

    if (SDL_MUSTLOCK(surface))
        SDL_UnlockSurface(surface);

    printf("Detected angle: %f\n", skewAngle);
    return result;
}
