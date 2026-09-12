#pragma once
#include <SDL2/SDL.h>
#include "math.h"
#include "neurons.h"

struct Mat* InvertForwardPassGrayScaleMatrix(struct Mat* grayScale);

// produce an array of SDL_Rect corresponding to the dimensions of each horizontal black lines on the grayScale (height is always set to 0), it also ignores little lines
SDL_Rect* ScanHorizontalLines(struct Mat* grayScale, size_t* lineCount_);
SDL_Rect* ConvertHorizontalLinesToBlocks(SDL_Rect* lines, size_t lineCount, size_t* blockCount_); // gather the adjacent lines produced by ScanHorizontalLines to form blocks

SDL_Rect* ScanVerticalLines(struct Mat* grayScale, size_t* lineCount_);
SDL_Rect* ConvertVerticalLinesToBlocks(SDL_Rect* lines, size_t lineCount, size_t* blockCount_); // gather the adjacent lines produced by ScanVerticalLines to form blocks

int SortBlocks(SDL_Rect** horizontalBlocks, SDL_Rect** verticalBlocks, size_t* horizontalBlockCount, size_t* verticalBlockCount);
// return a two dimentional array of the rectangles of each digit in the grid.
// IMPORTANT: this function supposes that horizontalBlocks and verticalBlocks represent a valid sudoku grid
SDL_Rect* GetSudokuDigitRects(SDL_Rect* horizontalBlocks, SDL_Rect* verticalBlocks);
SDL_Texture** GetSudokuDigitTextures(SDL_Rect* digitRects, char* sudokuFilePath);
struct Mat** ConvertTexturesToGrayScale(SDL_Texture** textures, size_t textureCount);
int DrawRects(SDL_Rect* rects, size_t rectCount, struct Mat* referenceMatrix, SDL_Color color); // draw the list of rectangles with the given color on the screen, referenceMatrix is needed to resize the rectangles correctly
int DrawFilledRects(SDL_Rect* rects, size_t rectCount, struct Mat* referenceMatrix, SDL_Color color);
int DrawGrayScale(struct Mat* grayScale);
int DrawDigitGrayScales(struct Mat** digitGrayScales, SDL_Rect* rects, size_t rectCount, struct Mat* referenceMatrix); // the digits given to the network are represented as column matrices, so we need a special function to display them
struct Mat* GetGridGrayScaleMatrix(char* imgFileName);// loads the given image and returns a matrix of its grayscale
// does the exact same as GetGrayScaleMatrix but stores the result in a column matrix so that it can be used for the network
struct Mat* GetForwardPassGrayScaleMatrix(SDL_Surface* trainingSurface);
