#pragma once
#include "math.h"
#include "neurons.h"

// debug functions -------
// the digits given to the network are represented as column matrices, so we need a special function to display them
int DrawDigitGrayScales(struct Mat** digitGrayScales, SDL_Rect* rects, size_t rectCount, struct Mat* referenceMatrix);


SDL_Surface *DeskewSurface(SDL_Surface *grayscale); // rotate the image


struct Mat* InvertGrayScaleMatrix(struct Mat* grayScale); // invert a grayScale (black = 0 becomes white = 1)

// produce an array of SDL_Rect corresponding to the dimensions of each horizontal black lines on the grayScale (height is always set to 0), it also ignores little lines
SDL_Rect* ScanHorizontalLines(struct Mat* grayScale, size_t* lineCount_);
SDL_Rect* ConvertHorizontalLinesToBlocks(SDL_Rect* lines, size_t lineCount, size_t* blockCount_); // gather the adjacent lines produced by ScanHorizontalLines to form blocks

SDL_Rect* ScanVerticalLines(struct Mat* grayScale, size_t* lineCount_); // same principle as ScanHorizontalLines
SDL_Rect* ConvertVerticalLinesToBlocks(SDL_Rect* lines, size_t lineCount, size_t* blockCount_); // same principle as ConvertHorizontalLinesToBlocks
// remove all the blocks that are not in a grid and change horizontalBlockCount and verticalBlockCount accordingly
int SortBlocks(SDL_Rect** horizontalBlocks, SDL_Rect** verticalBlocks, size_t* horizontalBlockCount, size_t* verticalBlockCount);
// IMPORTANT: this function supposes that horizontalBlocks and verticalBlocks represent a valid sudoku grid
SDL_Rect* GetSudokuDigitRects(SDL_Rect* horizontalBlocks, SDL_Rect* verticalBlocks); // return the array of rectangles from where to extract the characters on the grid
SDL_Texture** GetSudokuDigitTextures(SDL_Rect* digitRects, SDL_Texture* sudokuTexture); // extract the textures at the specified rectangles
struct Mat** ConvertTexturesToGrayScale(SDL_Texture** textures, size_t textureCount); // convert the list of textures to a list of grayScale and return it
int DrawRects(SDL_Rect* rects, size_t rectCount, struct Mat* referenceMatrix, SDL_Color color); // draw the list of rectangles with the given color on the screen, referenceMatrix is needed to resize the rectangles correctly
int DrawFilledRects(SDL_Rect* rects, size_t rectCount, struct Mat* referenceMatrix, SDL_Color color); // same as DrawRects but this time it draws filled rectangles
int DrawGrayScale(struct Mat* grayScale); // draw a grayScale on the screen
struct Mat* ConvertSurfaceToGrayScaleMatrix(SDL_Surface* surface);// convert the given surface into a grayscale matrix containing only 0s or 1s
SDL_Surface* ConvertSurfaceToGrayScale(SDL_Surface* surface); // convert the given surface to a grayscale, this grayScale will only contain 1s or 0s
// get the grayscale of the given image with values between 0 and 1 and store the result in a column matrix so that it can be used for the network
struct Mat* GetForwardPassGrayScaleMatrix(SDL_Surface* trainingSurface);
struct Mat** DeleteBlankGrayScales(struct Mat** grayScales, size_t grayScaleCount); // the grayScales considered empty in grayScales are set to NULL
