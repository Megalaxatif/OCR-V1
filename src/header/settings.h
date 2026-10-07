#pragma once

#define SCREEN_W 1000
#define SCREEN_H 1000
#define NETWORK_IMG_SIZE 28 // pixel width and height of the images that the network can take
#define GRID_OFFSET 3 // the rectangle of a digit in the grid might include part of the grid so we add this offset on each side of the rectangle to prevent that
#define MATRIX_BLANKNESS_THRESHOLD 0.98 // percentage of values close to 1 we must have in a matrix to say that it's blank
#define MATRIX_VALUE_BLANKNESS_THRESHOLD 0.95 // considering that every value in the matrix is between 0 and 1, if a pixel has a value superior to that number we consider it as a 1 (blank)
#define TRAIN_DIRECTORY_PATH "/home/megalaxatif/Documents/code/OCR-V1/database/train5/" // path where the train directory containing all the training images is in the project


#define NETWORK_PATH_BUFFER_SIZE 128 // size of the buffer containing the path we want to use to either store or load the network

#define DEFAULT_DEBUG_INFO 0
#define DEFAULT_NETWORK_PATH "save.ocr"
#define DEFAULT_TRAINING_CYCLE_COUNT 200
#define DEFAULT_LEARNING_RATE 0.02
#define MAXIMUM_LAYER_COUNT 6
#define DEFAULT_LAYER_COUNT 3
#define DEFAULT_NEURONS_PER_LAYER {NETWORK_IMG_SIZE * NETWORK_IMG_SIZE, 10, 10}
