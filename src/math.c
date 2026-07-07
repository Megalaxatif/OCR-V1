#include "header/math.h"

double Relu(double x){
    if (x < 0) return 0;
    return x;
}

double ReluPrime(double x){
    if (x < 0) return 0;
    return 1;
}



struct Mat* MatCreate(size_t row, size_t col, double* data[]){
    if (row <= 0 || col <= 0) {
        printf("Error: MatCreate, invalid row or col value\n");
        return NULL;
    }

    struct Mat* newMat = malloc(sizeof(struct Mat));

    newMat->data = calloc(sizeof(double*), row);
    newMat->row = row;
    newMat->col = col;

    for(int i = 0; i < row; i++){
        newMat->data[i] = calloc(sizeof(double), col);
    }

    if (data != NULL){
        for(size_t i = 0; i < newMat->row; i++){
            for(size_t j = 0; j < newMat->col; j++){
                newMat->data[i][j] = data[i][j];
            }
        }
    }
    return newMat;
}

void MatDestroy(struct Mat* mat){
    if (mat == NULL) return;
    if (mat->data != NULL && *(mat->data) != NULL){
        for(size_t i = 0; i < mat->row; i++){
            free(mat->data[i]);
        }
        free(mat->data);
    }
    free(mat);
}

struct Mat* MatTranspose(struct Mat* mat){
    struct Mat* transpose = MatCreate(mat->col, mat->row, NULL);
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


struct Mat* MatAdd(struct Mat* mat_1, struct Mat* mat_2){ // add mat_2 to mat_1 and return mat_1
    if (mat_1->col != mat_2->col || mat_1->row != mat_2->row){
        printf("Error: MatAdd, the matrices don't have the same size\n");
        return NULL;
    }
    for(size_t i = 0; i < mat_1->row; i++){
        for(size_t j = 0; j < mat_1->col; j++){
            mat_1->data[i][j] += mat_2->data[i][j];
        }
    }
    return mat_1;
}


struct Mat* MatHadamard(struct Mat* mat_1, struct Mat* mat_2){ // perform the Hadamard product between mat_1 and mat_2 and return mat_1
    if (mat_1->col != mat_2->col || mat_1->row != mat_2->row){
        printf("Error: MatHadamard, the matrices don't have the same size\n");
        return NULL;
    }
    for(size_t i = 0; i < mat_1->row; i++){
        for(size_t j = 0; j < mat_1->col; j++){
            mat_1->data[i][j] *= mat_2->data[i][j];
        }
    }
    return mat_1;
}


struct Mat* MatMult(struct Mat* mat_1, struct Mat* mat_2){ // perform the matrix multiplication between mat_1 and mat_2 and return the resulting matrix
    if (mat_1->col != mat_2->row){
        printf("Error: MatMult, the matrices don't have compatible size\n");
        return NULL;
    }
    struct Mat* res = MatCreate(mat_1->row, mat_2->col, NULL);
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

struct Mat* MatFunc(struct Mat* mat_1, double (*f)(double)){ // apply the function f on every element of mat_1 and return it
    for(size_t i = 0; i < mat_1->row; i++){
        for(size_t j = 0; j < mat_1->col; j++){
            mat_1->data[i][j] = f(mat_1->data[i][j]);
        }
    }
    return mat_1;
}
