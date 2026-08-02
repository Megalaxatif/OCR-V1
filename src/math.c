#include "header/math.h"
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include "header/init.h"

size_t matCount = 0;

int Softmax(const struct Mat *input, struct Mat *output){
    if (input->col != 1 || output->col != 1){
        printf("Error: Softmax, input or output are not a column matrix\n");
        return 1;
    }

    if (input->row != output->row){
        printf("Error: Softmax, input and output don't have the same number of rows\n");
        return 2;
    }

    double max = input->data[0][0];

    for (size_t i = 1; i < input->row; i++){
        if (input->data[i][0] > max)
            max = input->data[i][0];
    }

    double sum = 0.0;

    for (size_t i = 0; i < input->row; i++){
        output->data[i][0] = exp(input->data[i][0] - max);
        sum += output->data[i][0];
    }

    for (size_t i = 0; i < input->row; i++){
        output->data[i][0] /= sum;
    }
    return 0;
}

double Random(double min, double max){
    return min + (max - min) * ((double)rand() / RAND_MAX);
}

size_t RandomInt(size_t min, size_t max){
    return min + (max - min) * ((size_t)rand() / RAND_MAX);
}

void InitBiases(struct Mat* B){
    for (size_t i = 0; i < B->row; i++)
        for (size_t j = 0; j < B->col; j++)
            B->data[i][j] = Random(-0.01, 0.01);
}

void InitWeights(struct Mat *W){
    double limit = sqrt(6.0 / W->col);

    for (size_t i = 0; i < W->row; i++)
        for (size_t j = 0; j < W->col; j++)
            W->data[i][j] = Random(-limit, limit);
}

double Relu(double x){
    if (x < 0) return 0;
    return x;
}

double ReluPrime(double x){
    if (x < 0) return 0;
    return 1;
}


struct Mat* MatCreate(size_t row, size_t col, double* data[], void (*f)(struct Mat* mat)){
    if (row <= 0 || col <= 0) {
        printf("Error: MatCreate, invalid row or col value\n");
        return NULL;
    }

    struct Mat* newMat = malloc(sizeof(struct Mat));

    newMat->data = malloc(row*sizeof(double*));
    newMat->row = row;
    newMat->col = col;

    for(int i = 0; i < row; i++){
        newMat->data[i] = malloc(col*sizeof(double));
    }

    if (data != NULL){
        for(size_t i = 0; i < newMat->row; i++){
            for(size_t j = 0; j < newMat->col; j++){
                newMat->data[i][j] = data[i][j];
            }
        }
    }
    else if (f != NULL)
        f(newMat);

    matCount++;
    return newMat;
}

void MatDestroy(struct Mat* mat){
    if (mat == NULL) return;
    if (mat->data != NULL){
        if (*(mat->data) != NULL){
            for(size_t i = 0; i < mat->row; i++){
                free(mat->data[i]);
            }
            free(mat->data);
        }
    }
    free(mat);
    matCount--;
}

struct Mat* MatCopy(struct Mat* mat){
    if (mat == NULL){
        printf("Error: MatCopy, mat is NULL\n");
        return NULL;
    }
    struct Mat* copy = MatCreate(mat->row, mat->col, mat->data, NULL);
    return copy;
}

struct Mat* MatTranspose(struct Mat* mat){
    if (mat == NULL){
        printf("Error: MatTranspose, mat is NULL\n");
    }
    struct Mat* transpose = MatCreate(mat->col, mat->row, NULL, NULL);
    for(size_t i = 0; i < mat->row; i++){
        for(size_t j = 0; j < mat->col; j++){
            transpose->data[j][i] = mat->data[i][j];
        }
    }
    return transpose;
}


void MatPrint(struct Mat* mat){
    if (mat == NULL){
        printf("Error: MatPrint, matrix pointer is NULL\n");
        return;
    }
    for(size_t i = 0; i < mat->row; i++){
        for(size_t j = 0; j < mat->col; j++){
            printf("%.1f ", mat->data[i][j]);
        }
        printf("\n");
    }
}


struct Mat* MatAdd(struct Mat* mat_1, struct Mat* mat_2){
    if (mat_1 == NULL){
        printf("Error: MatAdd, mat_1 is NULL\n");
        return NULL;
    }
    if (mat_2 == NULL){
        printf("Error: MatAdd, mat_2 is NULL\n");
        return NULL;
    }
    if (mat_1->col != mat_2->col || mat_1->row != mat_2->row){
        printf("Error: MatAdd, the matrices don't have the same size\n");
        return NULL;
    }
    struct Mat* newMat = MatCreate(mat_1->row, mat_1->col, NULL, NULL);
    for(size_t i = 0; i < newMat->row; i++){
        for(size_t j = 0; j < newMat->col; j++){
            newMat->data[i][j] = mat_1->data[i][j] + mat_2->data[i][j];
        }
    }
    return newMat;
}

int MatAddInternal(struct Mat* mat_1, struct Mat* mat_2){
    if (mat_1 == NULL){
        printf("Error: MatAddInternal, mat_1 is NULL\n");
        return 1;
    }
    if (mat_2 == NULL){
        printf("Error: MatAddInternal, mat_2 is NULL\n");
        return 2;
    }
    if (mat_1->col != mat_2->col || mat_1->row != mat_2->row){
        printf("Error: MatAddInternal, the matrices don't have the same size\n");
        return 3;
    }
    for(size_t i = 0; i < mat_1->row; i++){
        for(size_t j = 0; j < mat_1->col; j++){
            mat_1->data[i][j] += mat_2->data[i][j];
        }
    }
    return 0;
}

struct Mat* MatSub(struct Mat* mat_1, struct Mat* mat_2){
    if (mat_1->col != mat_2->col || mat_1->row != mat_2->row){
        printf("Error: MatSub, the matrices don't have the same size\n");
        return NULL;
    }

    struct Mat* newMat = MatCreate(mat_1->row, mat_1->col, NULL, NULL);
    for(size_t i = 0; i < newMat->row; i++){
        for(size_t j = 0; j < newMat->col; j++){
            newMat->data[i][j] = mat_1->data[i][j] - mat_2->data[i][j];
        }
    }
    return newMat;
}

struct Mat* MatSubInternal(struct Mat* mat_1, struct Mat* mat_2){
    if (mat_1->col != mat_2->col || mat_1->row != mat_2->row){
        printf("Error: MatSubInternal, the matrices don't have the same size\n");
        return NULL;
    }

    for(size_t i = 0; i < mat_1->row; i++){
        for(size_t j = 0; j < mat_1->col; j++){
            mat_1->data[i][j] -= mat_2->data[i][j];
        }
    }
    return mat_1;
}


struct Mat* MatHadamard(struct Mat* mat_1, struct Mat* mat_2){
    if (mat_1->col != mat_2->col || mat_1->row != mat_2->row){
        printf("Error: MatHadamard, the matrices don't have the same size\n");
        return NULL;
    }
    struct Mat* newMat = MatCreate(mat_1->row, mat_1->col, NULL, NULL);
    for(size_t i = 0; i < newMat->row; i++){
        for(size_t j = 0; j < newMat->col; j++){
            newMat->data[i][j] = mat_1->data[i][j] * mat_2->data[i][j];
        }
    }
    return newMat;
}

struct Mat* MatHadamardInternal(struct Mat* mat_1, struct Mat* mat_2){
    if (mat_1->col != mat_2->col || mat_1->row != mat_2->row){
        printf("Error: MatHadamardInternal, the matrices don't have the same size\n");
        return NULL;
    }
    for(size_t i = 0; i < mat_1->row; i++){
        for(size_t j = 0; j < mat_1->col; j++){
            mat_1->data[i][j] *= mat_2->data[i][j];
        }
    }
    return mat_1;
}


struct Mat* MatMult(struct Mat* mat_1, struct Mat* mat_2){
    if (mat_1->col != mat_2->row){
        printf("Error: MatMult, the matrices don't have compatible size\n");
        return NULL;
    }
    struct Mat* res = MatCreate(mat_1->row, mat_2->col, NULL, NULL);
    for(size_t j = 0; j < mat_2->col; j++){
        for(size_t i = 0; i < mat_1->row; i++){
            double tmp = 0;
            for(int k = 0; k < mat_2->row; k++){
                tmp += mat_1->data[i][k] * mat_2->data[k][j];
            }
            res->data[i][j] = tmp;
        }
    }
    return res;
}

struct Mat* MatFunc(struct Mat* mat, double (*f)(double)){
    if (mat == NULL){
        printf("Error: MatFunc, the matrix is NULL\n");
        return NULL;
    }
    if (f == NULL){
        printf("Error: MatFunc, the function is NULL\n");
        return NULL;
    }
    struct Mat* newMat = MatCreate(mat->row, mat->col, NULL, NULL);
    for(size_t i = 0; i < newMat->row; i++){
        for(size_t j = 0; j < newMat->col; j++){
            newMat->data[i][j] = f(mat->data[i][j]);
        }
    }
    return newMat;
}

struct Mat* MatFuncInternal(struct Mat* mat, double (*f)(double)){
    if (mat == NULL){
        printf("Error: MatFuncInternal, the matrix is NULL\n");
        return NULL;
    }
    if (f == NULL){
        printf("Error: MatFuncInternal, the function is NULL\n");
        return NULL;
    }
    for(size_t i = 0; i < mat->row; i++){
        for(size_t j = 0; j < mat->col; j++){
            mat->data[i][j] = f(mat->data[i][j]);
        }
    }
    return mat;
}

struct Mat* MatScalar(struct Mat* mat, double scalar){
    if (mat == NULL){
        printf("Error: MatScalar, the matrix is NULL\n");
        return NULL;
    }
    struct Mat* newMat = MatCreate(mat->row, mat->col, NULL, NULL);
    for(size_t i = 0; i < newMat->row; i++){
        for(size_t j = 0; j < newMat->col; j++){
            newMat->data[i][j] = scalar * mat->data[i][j];
        }
    }
    return newMat;
}

struct Mat* MatScalarInternal(struct Mat* mat, double scalar){
    if (mat == NULL){
        printf("Error: MatScalarInternal, the matrix is NULL\n");
        return NULL;
    }
    for(size_t i = 0; i < mat->row; i++){
        for(size_t j = 0; j < mat->col; j++){
            mat->data[i][j] *= scalar;
        }
    }
    return mat;
}
