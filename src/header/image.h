#pragma once

struct Mat* GetGrayScaleMatrix(char* imgFileName);
// does the exact same as GetGrayScaleMatrix but stores the result in a column matrix so that it can be used for the network
struct Mat* GetTrainingGrayScaleMatrix(char imgFileName[]);
int DrawGrayScale(struct Mat* grayScale);
