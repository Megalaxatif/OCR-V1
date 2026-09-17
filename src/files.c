#include "header/files.h"
#include "header/init.h"
#include "header/math.h"
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

int GetSample10(char*** sample, char*** files, size_t* fileCount){
    if (sample == NULL || files == NULL || fileCount == NULL){
        printf("Error: GetSample10, invalid argument\n");
        return 1;
    }

    int len = strlen(TRAIN_DIRECTORY_PATH);
    for(int i = 0; i < 10; i++){
        char* currentPath = (*sample)[i];
        strcpy(currentPath, TRAIN_DIRECTORY_PATH);
        currentPath[len] = i + '0';
        currentPath[len + 1] = '/';
        currentPath[len + 2] = '\0';
        size_t index = RandomInt(0, fileCount[i]-1);
        strcat(currentPath, files[i][index]);
    }
    return 0;
}

char*** GetAllTrainingFileNames(size_t** _fileCount){
    if (_fileCount == NULL){
        printf("Error: GetAllTrainingFileNames, _fileCount is NULL\n");
        return NULL;
    }
    char*** files = malloc(10*sizeof(char**)); // files[i] correspond to the list of training image path for the digit i
    size_t* fileCount = malloc(10*sizeof(size_t)); // fileCount[i] correspond to the number of files in files[i]

    char path[100];
    strcpy(path, TRAIN_DIRECTORY_PATH);
    size_t pathLength = strlen(path);

    for(int i = 0; i < 10; i++){
        path[pathLength] = i + '0';
        path[pathLength + 1] = '\0';
        files[i] = GetFileNames(path, fileCount + i);

        if (files[i] == NULL){
            printf("Error: GetAllTrainingFileNames, GetFileNames returned NULL at some point\n");
            // free the ressources
            for(int k = 0; k < i; k++){
                for(size_t j = 0; j < fileCount[k]; j++){
                    free(files[k][j]);
                }
                free(files[k]);
            }
            free(files);
            free(fileCount);
            return NULL;
        }
    }
    *_fileCount = fileCount;
    return files;
}

char** GetFileNames(char* dirPath, size_t* _fileCount){
    if (dirPath == NULL){
        printf("Error: GetFileNames, dirPath is NULL\n");
        return NULL;
    }
    if (_fileCount == NULL){
        printf("Error: GetFileNames, _fileCount is NULL\n");
        return NULL;
    }
    DIR *dir = opendir(dirPath);

    if (dir == NULL){
        printf("Error: getFileNames, invalid dirPath\n");
        return NULL;
    }

    size_t fileCount = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL){
        if (entry->d_type == DT_REG){
            fileCount++;
        }
    }
    char** files = malloc(sizeof(char*)*fileCount);
    rewinddir(dir); // go back to the beginning
    int i = 0;
    while ((entry = readdir(dir)) != NULL){
        if (entry->d_type == DT_REG){
            files[i] = strdup(entry->d_name);
            i++;
        }
    }
    *_fileCount = fileCount;
    closedir(dir);
    return files;
}

int SaveMatrix(struct Mat* mat, char* path){
    printf("saving matrix\n");
    if (mat == NULL || path == NULL){
        printf("Error: SaveMatrix, invalidArgument\n");
        return 1;
    }
    FILE* file = fopen(path, "a");
    if (file == NULL){
        printf("Error: SaveMatrix, impossible to open the file %s\n", path);
        return 2;
    }
    fprintf(file, "matrix:{\n\trow:%ld\n\tcol:%ld\n\tdata:{\n\t\t", mat->row, mat->col);
    for(size_t y = 0; y < mat->row; y++){
        for(size_t x = 0; x < mat->col; x++){
            fprintf(file, "%.17f;", mat->data[y][x]);
        }
        y + 1 == mat->row ? fprintf(file, "\n\t}\n}\n") : fprintf(file, "\n\t\t");
    }
    fclose(file);
    return 0;
}

struct Mat* LoadMatrix(FILE* file){
    printf("loading matrix\n");
    if (file == NULL){
        printf("Error: LoadMatrix, invalidArgument\n");
        return NULL;
    }
    struct Mat* ret = NULL;
    char* lineBuffer = NULL;
    size_t lineBufferSize = 0;
    int matrixFound = 0;

    while(getline(&lineBuffer, &lineBufferSize, file) != EOF && !matrixFound){
        //printf("line: %s", lineBuffer);
        if (strstr(lineBuffer, "matrix")){
            matrixFound = 1;
        }
    }
    printf("line 1: %s", lineBuffer);

    if (matrixFound){
        size_t row = 0;
        size_t col = 0;
        char* cursor = NULL;
        // row
        printf("line 2 : %s", lineBuffer);
        if (getline(&lineBuffer, &lineBufferSize, file) == EOF){
            printf("Error: LoadMatrix, the matrix is cut in half before the row argument\n");
            goto cleanup;
        }
        printf("line 3: %s", lineBuffer);
        if ((cursor = strstr(lineBuffer, "row:")) == NULL){
            printf("Error: LoadMatrix, the matrix doesn't have a row argument\n");
            goto cleanup;
        }
        if (sscanf(cursor, "row:%ld", &row) != 1){
            printf("Error: LoadMatrix, the row argument is empty or invalid\n");
            goto cleanup;
        }
        // col
        if (!getline(&lineBuffer, &lineBufferSize, file)){
            printf("Error: LoadMatrix, the matrix is cut in half before the col argument\n");
            goto cleanup;
        }
        if (!(cursor = strstr(lineBuffer, "col:"))){
            printf("Error: LoadMatrix, the matrix doesn't have a col argument\n");
            goto cleanup;
        }
        if (sscanf(cursor, "col:%ld", &col) != 1){
            printf("Error: LoadMatrix, the col argument is empty or invalid\n");
            goto cleanup;
        }
        printf("row: %ld, col: %ld\n", row, col);
    }
    else {
        printf("WARNING: LoadMatrix, no more matrices found in the file\n");
        goto cleanup;
    }

    cleanup:
    free(lineBuffer);
    return NULL;
}
