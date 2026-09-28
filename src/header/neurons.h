#pragma once
#include "math.h"
#include "ocr.h"

struct Layer* CreateLayer(size_t currentLayerNeuronCount, size_t nextLayerNeuronCount, struct Mat* weights, struct Mat* biases); // explicit enough
void DestroyLayer(struct Layer* layer); // you can't be more explicit than that
int ForwardPass(struct Network* network, struct Mat* input);
int GetGuessedDigit(struct Network* network);
int Train(struct Network* network, char** sample, size_t sampleSize, struct Mat* answer[]);
struct Mat** GetAnswer10(); // get the list of answer matrix for a training of 1 image on each digit from 0 to 9
struct Network* CreateNetwork(double learningRate, size_t layerCount, int* neuronsPerLayer,  struct Mat* weights[], struct Mat* biases[]);
void DestroyNetwork(struct Network* network);
void PrintDigitGrayScales(struct Mat** digitGrayScale, size_t grayScaleCount);
int* SolveGrayScales(struct Mat** grayScales, size_t grayScaleCount, struct Network* network);
int SolveSudoku(struct OCR* ocr);
int SolveImage(char* path, struct Network* network, int isTraining);
struct Network* LoadNetwork(char* path);
int SaveNetwork(struct Network* network, char* path);
