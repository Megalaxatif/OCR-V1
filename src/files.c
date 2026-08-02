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

    char path[50];
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
