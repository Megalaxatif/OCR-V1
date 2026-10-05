#include <SDL2/SDL.h>
#include <math.h>
#include <stdlib.h>

#define PI 3.14159265358979323846
#define MAX_SKEW_ANGLE 15.0
#define ANGLE_STEP 0.25
#define EDGE_THRESHOLD 120

static Uint8 GetGrayRGBA32(SDL_Surface *surface, int x, int y)
{
    Uint32 *pixels = (Uint32 *)surface->pixels;
    int pitch = surface->pitch / 4;

    Uint32 pixel = pixels[y * pitch + x];

    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;

    SDL_GetRGBA(pixel, surface->format, &r, &g, &b, &a);

    /*
     * The image is already grayscale, so R = G = B.
     * Reading one channel is enough.
     */
    return r;
}

static void SetGrayRGBA32(SDL_Surface *surface, int x, int y, Uint8 value)
{
    Uint32 *pixels = (Uint32 *)surface->pixels;
    int pitch = surface->pitch / 4;

    /*
     * Write the same value to R, G and B.
     * Alpha is kept fully opaque.
     */
    pixels[y * pitch + x] =
        SDL_MapRGBA(surface->format, value, value, value, 255);
}

static Uint8 *ExtractGrayBuffer(SDL_Surface *grayscale)
{
    int width = grayscale->w;
    int height = grayscale->h;

    Uint8 *buffer = malloc((size_t)width * height);

    if (buffer == NULL)
        return NULL;

    if (SDL_MUSTLOCK(grayscale))
        SDL_LockSurface(grayscale);

    /*
     * Extract one grayscale intensity value per pixel.
     */
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
            buffer[y * width + x] = GetGrayRGBA32(grayscale, x, y);
    }

    if (SDL_MUSTLOCK(grayscale))
        SDL_UnlockSurface(grayscale);

    return buffer;
}

static Uint8 *CreateEdgeImage(const Uint8 *gray, int width, int height)
{
    Uint8 *edges = calloc((size_t)width * height, sizeof(Uint8));

    if (edges == NULL)
        return NULL;

    /*
     * Apply a Sobel operator to detect strong intensity changes.
     */
    for (int y = 1; y < height - 1; y++)
    {
        for (int x = 1; x < width - 1; x++)
        {
            int gx =
                -gray[(y - 1) * width + (x - 1)]
                + gray[(y - 1) * width + (x + 1)]
                - 2 * gray[y * width + (x - 1)]
                + 2 * gray[y * width + (x + 1)]
                - gray[(y + 1) * width + (x - 1)]
                + gray[(y + 1) * width + (x + 1)];

            int gy =
                -gray[(y - 1) * width + (x - 1)]
                - 2 * gray[(y - 1) * width + x]
                - gray[(y - 1) * width + (x + 1)]
                + gray[(y + 1) * width + (x - 1)]
                + 2 * gray[(y + 1) * width + x]
                + gray[(y + 1) * width + (x + 1)];

            int magnitude = abs(gx) + abs(gy);

            /*
             * Keep only sufficiently strong edges.
             */
            if (magnitude >= EDGE_THRESHOLD)
                edges[y * width + x] = 1;
        }
    }

    return edges;
}

static int GetTopScore(const int *accumulator, int rhoCount)
{
    const int peakCount = 10;
    int best[10] = {0};

    /*
     * Keep the strongest Hough peaks.
     */
    for (int r = 0; r < rhoCount; r++)
    {
        int value = accumulator[r];

        for (int i = 0; i < peakCount; i++)
        {
            if (value > best[i])
            {
                for (int j = peakCount - 1; j > i; j--)
                    best[j] = best[j - 1];

                best[i] = value;
                break;
            }
        }
    }

    int score = 0;

    for (int i = 0; i < peakCount; i++)
        score += best[i];

    return score;
}

static double FindSkewAngle(const Uint8 *edges, int width, int height)
{
    int diagonal = (int)ceil(
        sqrt((double)width * width +
             (double)height * height));

    int rhoCount = diagonal * 2 + 1;

    int *horizontalAccumulator =
        malloc((size_t)rhoCount * sizeof(int));

    int *verticalAccumulator =
        malloc((size_t)rhoCount * sizeof(int));

    if (horizontalAccumulator == NULL ||
        verticalAccumulator == NULL)
    {
        free(horizontalAccumulator);
        free(verticalAccumulator);
        return 0.0;
    }

    double bestAngle = 0.0;
    int bestScore = -1;

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

        double rad = angle * PI / 180.0;

        double cosV = cos(rad);
        double sinV = sin(rad);

        double horizontalRad = rad + PI / 2.0;
        double cosH = cos(horizontalRad);
        double sinH = sin(horizontalRad);

        /*
         * Vote in the Hough accumulators for every edge pixel.
         */
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                if (!edges[y * width + x])
                    continue;

                double rhoV = x * cosV + y * sinV;
                double rhoH = x * cosH + y * sinH;

                int indexV = (int)round(rhoV) + diagonal;
                int indexH = (int)round(rhoH) + diagonal;

                if (indexV >= 0 && indexV < rhoCount)
                    verticalAccumulator[indexV]++;

                if (indexH >= 0 && indexH < rhoCount)
                    horizontalAccumulator[indexH]++;
            }
        }

        int verticalScore =
            GetTopScore(verticalAccumulator, rhoCount);

        int horizontalScore =
            GetTopScore(horizontalAccumulator, rhoCount);

        int score = verticalScore + horizontalScore;

        /*
         * Keep the angle producing the strongest line peaks.
         */
        if (score > bestScore)
        {
            bestScore = score;
            bestAngle = angle;
        }
    }

    free(horizontalAccumulator);
    free(verticalAccumulator);

    return bestAngle;
}

static SDL_Surface *RotateGrayRGBA32(SDL_Surface *source, double angle)
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
        fabs(srcHeight * s));

    int dstHeight = (int)ceil(
        fabs(srcWidth * s) +
        fabs(srcHeight * c));

    SDL_Surface *destination =
        SDL_CreateRGBSurfaceWithFormat(
            0,
            dstWidth,
            dstHeight,
            32,
            SDL_PIXELFORMAT_RGBA32);

    if (destination == NULL)
        return NULL;

    /*
     * Fill empty areas with white.
     */
    SDL_FillRect(
        destination,
        NULL,
        SDL_MapRGBA(destination->format, 255, 255, 255, 255));

    double srcCX = (srcWidth - 1) / 2.0;
    double srcCY = (srcHeight - 1) / 2.0;

    double dstCX = (dstWidth - 1) / 2.0;
    double dstCY = (dstHeight - 1) / 2.0;

    if (SDL_MUSTLOCK(source))
        SDL_LockSurface(source);

    if (SDL_MUSTLOCK(destination))
        SDL_LockSurface(destination);

    /*
     * Use inverse mapping:
     * for each destination pixel, compute its source position.
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

            Uint8 value =
                GetGrayRGBA32(source, sx, sy);

            SetGrayRGBA32(
                destination,
                x,
                y,
                value);
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
        SDL_SetError("DeskewSurface: input surface is NULL");
        return NULL;
    }

    /*
     * The input surface must use RGBA32.
     */
    if (grayscale->format == NULL ||
        grayscale->format->format != SDL_PIXELFORMAT_RGBA32)
    {
        SDL_SetError(
            "DeskewSurface: input surface must use SDL_PIXELFORMAT_RGBA32");
        return NULL;
    }

    Uint8 *gray = ExtractGrayBuffer(grayscale);

    if (gray == NULL)
    {
        SDL_SetError(
            "DeskewSurface: failed to allocate grayscale buffer");
        return NULL;
    }

    /*
     * Detect image edges.
     */
    Uint8 *edges = CreateEdgeImage(
        gray,
        grayscale->w,
        grayscale->h);

    free(gray);

    if (edges == NULL)
    {
        SDL_SetError(
            "DeskewSurface: failed to allocate edge buffer");
        return NULL;
    }

    /*
     * Estimate the skew angle using the Hough transform.
     */
    double angle = FindSkewAngle(
        edges,
        grayscale->w,
        grayscale->h);

    free(edges);

    /*
     * Rotate in the opposite direction to correct the skew.
     */
    return RotateGrayRGBA32(grayscale, -angle);
}
