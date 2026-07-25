#pragma once
#include <stddef.h>

// return a dynamically allocated array containing all the name of the files in the directory and stores the number of files in the directory in fileCount
char** GetFileNames(char* dirPath, size_t* fileCount);
// return a dynamically allocated array contatining all the file names of each directory in the training folder and stores the number of files in each directory in fileCount
char*** GetAllTrainingFileNames(size_t** fileCount);
