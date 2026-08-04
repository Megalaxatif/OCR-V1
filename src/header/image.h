#pragma once
#include <SDL2/SDL.h>
#include "math.h"
// return a dynamically allocated array of SDL_Point where (array[i], array[i+1]) correspond to the 2 ends of an horizontal line on the image, it also ignores little lines
SDL_Point* ScanHorizontalLines(struct Mat* grayScale, size_t* pointCount);
int DrawHorizontalLines(SDL_Point* horizontalLines, size_t pointCount);
struct Mat* GetGridGrayScaleMatrix(char* imgFileName);
// does the exact same as GetGrayScaleMatrix but stores the result in a column matrix so that it can be used for the network
struct Mat* GetTrainingGrayScaleMatrix(char imgFileName[]);
int DrawGrayScale(struct Mat* grayScale);
