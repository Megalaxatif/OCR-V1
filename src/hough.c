#include <SDL2/SDL.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAX_SKEW_ANGLE 90
#define ROUGH_ANGLE_STEP 1.0
#define FINE_ANGLE_STEP 0.1
#define SAMPLE_STEP 2

typedef struct
{
    int x;
    int y;
} Point;

static Uint8 getBinaryValue(
    const SDL_Surface *surface,
    int x,
    int y)
{
    const Uint8 *row =
        (const Uint8 *)surface->pixels +
        (size_t)y * surface->pitch;

    /*
     * SDL_PIXELFORMAT_RGBA32 garantit l'ordre mémoire :
     *
     * R G B A
     *
     * L'image étant binarisée, tester R suffit.
     */
    return row[x * 4] == 0 ? 0 : 1;
}

static Uint8 findForegroundValue(
    const SDL_Surface *surface)
{
    size_t blackCount = 0;
    size_t whiteCount = 0;

    for (int y = 0; y < surface->h; y += SAMPLE_STEP)
    {
        for (int x = 0; x < surface->w; x += SAMPLE_STEP)
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

static uint64_t getAngleScore(
    const Point *points,
    size_t pointCount,
    double angle,
    int *accumulator,
    int rhoCount,
    int maxRho)
{
    memset(
        accumulator,
        0,
        (size_t)rhoCount * sizeof(int));

    double radians =
        angle * M_PI / 180.0;

    double cosAngle = cos(radians);
    double sinAngle = sin(radians);

    for (size_t i = 0; i < pointCount; i++)
    {
        double rho =
            (double)points[i].x * cosAngle +
            (double)points[i].y * sinAngle;

        int rhoIndex =
            (int)lround(rho) + maxRho;

        if (rhoIndex >= 0 &&
            rhoIndex < rhoCount)
        {
            accumulator[rhoIndex]++;
        }
    }

    uint64_t score = 0;

    for (int i = 0; i < rhoCount; i++)
    {
        uint64_t value =
            (uint64_t)accumulator[i];

        score += value * value;
    }

    return score;
}

static double findSkewAngle(
    const SDL_Surface *surface,
    Uint8 foreground)
{
    int width = surface->w;
    int height = surface->h;

    /*
     * Taille maximale du tableau.
     * Comme on prend un pixel tous les SAMPLE_STEP pixels.
     */
    size_t maxPointCount =
        ((size_t)(width + SAMPLE_STEP - 1) / SAMPLE_STEP) *
        ((size_t)(height + SAMPLE_STEP - 1) / SAMPLE_STEP);

    Point *points =
        malloc(maxPointCount * sizeof(Point));

    if (points == NULL)
        return 0.0;

    size_t pointCount = 0;

    /*
     * On récupère une seule fois tous les pixels appartenant
     * au contenu.
     *
     * Ensuite Hough travaille uniquement sur cette liste.
     */
    for (int y = 0; y < height; y += SAMPLE_STEP)
    {
        for (int x = 0; x < width; x += SAMPLE_STEP)
        {
            if (getBinaryValue(surface, x, y) == foreground)
            {
                points[pointCount].x = x;
                points[pointCount].y = y;

                pointCount++;
            }
        }
    }

    if (pointCount == 0)
    {
        free(points);
        return 0.0;
    }

    int maxRho =
        (int)ceil(
            hypot(
                (double)width,
                (double)height));

    int rhoCount =
        2 * maxRho + 1;

    int *accumulator =
        calloc(
            (size_t)rhoCount,
            sizeof(int));

    if (accumulator == NULL)
    {
        free(points);
        return 0.0;
    }

    double bestAngle = 0.0;
    uint64_t bestScore = 0;

    /*
     * Première passe :
     *
     * recherche grossière par pas de 1 degré.
     */
    for (double angle = -MAX_SKEW_ANGLE;
         angle <= MAX_SKEW_ANGLE;
         angle += ROUGH_ANGLE_STEP)
    {
        uint64_t score =
            getAngleScore(
                points,
                pointCount,
                angle,
                accumulator,
                rhoCount,
                maxRho);

        if (score > bestScore)
        {
            bestScore = score;
            bestAngle = angle;
        }
    }

    /*
     * Deuxième passe :
     *
     * recherche précise uniquement autour du meilleur angle.
     */
    double roughAngle = bestAngle;

    bestScore = 0;

    for (double angle = roughAngle - ROUGH_ANGLE_STEP;
         angle <= roughAngle + ROUGH_ANGLE_STEP;
         angle += FINE_ANGLE_STEP)
    {
        uint64_t score =
            getAngleScore(
                points,
                pointCount,
                angle,
                accumulator,
                rhoCount,
                maxRho);

        if (score > bestScore)
        {
            bestScore = score;
            bestAngle = angle;
        }
    }

    free(accumulator);
    free(points);

    return bestAngle;
}

static SDL_Surface *rotateBinaryRgba32(
    const SDL_Surface *source,
    double angle,
    Uint8 background)
{
    double radians =
        angle * M_PI / 180.0;

    double cosAngle = cos(radians);
    double sinAngle = sin(radians);

    int sourceWidth = source->w;
    int sourceHeight = source->h;

    int destinationWidth =
        (int)ceil(
            fabs((double)sourceWidth * cosAngle) +
            fabs((double)sourceHeight * sinAngle));

    int destinationHeight =
        (int)ceil(
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

    if (SDL_FillRect(
            destination,
            NULL,
            backgroundPixel) != 0)
    {
        SDL_FreeSurface(destination);
        return NULL;
    }

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

    /*
     * Mapping inverse.
     *
     * Chaque pixel de l'image de destination cherche sa position
     * correspondante dans l'image source.
     *
     * On utilise nearest-neighbor pour ne créer aucune valeur grise.
     */
    for (int y = 0; y < destinationHeight; y++)
    {
        Uint8 *destinationRow =
            (Uint8 *)destination->pixels +
            (size_t)y * destination->pitch;

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

            const Uint8 *sourceRow =
                (const Uint8 *)source->pixels +
                (size_t)sourcePixelY * source->pitch;

            const Uint8 *sourcePixel =
                sourceRow + sourcePixelX * 4;

            Uint8 *destinationPixel =
                destinationRow + x * 4;

            /*
             * Copie directement R, G, B, A.
             */
            destinationPixel[0] = sourcePixel[0];
            destinationPixel[1] = sourcePixel[1];
            destinationPixel[2] = sourcePixel[2];
            destinationPixel[3] = sourcePixel[3];
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
        SDL_SetError(
            "DeskewSurface: surface is NULL");

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

    SDL_Log(
        "DeskewSurface: detected angle = %.2f degrees",
        skewAngle);

    SDL_Surface *result =
        rotateBinaryRgba32(
            surface,
            -skewAngle,
            background);

    if (SDL_MUSTLOCK(surface))
        SDL_UnlockSurface(surface);

    return result;
}
