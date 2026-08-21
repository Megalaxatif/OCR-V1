#pragma once
#include <SDL2/SDL.h>
#include "math.h"
// produce an array of SDL_Rect corresponding to the dimensions of each horizontal black lines on the grayScale (height is always set to 0), it also ignores little lines
SDL_Rect* ScanHorizontalLines(struct Mat* grayScale, size_t* lineCount_);
SDL_Rect* ConvertHorizontalLinesToBlocks(SDL_Rect* lines, size_t lineCount, size_t* blockCount_); // gather the adjacent lines produced by ScanHorizontalLines to form blocs

SDL_Rect* ScanVerticalLines(struct Mat* grayScale, size_t* lineCount_);
SDL_Rect* ConvertVerticalLinesToBlocks(SDL_Rect* lines, size_t lineCount, size_t* blockCount_);

int DrawRect(SDL_Rect* rects, size_t rectCount);
struct Mat* GetGridGrayScaleMatrix(char* imgFileName);
// does the exact same as GetGrayScaleMatrix but stores the result in a column matrix so that it can be used for the network
struct Mat* GetTrainingGrayScaleMatrix(char imgFileName[]);
int DrawGrayScale(struct Mat* grayScale);
