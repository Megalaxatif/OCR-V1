#pragma once
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Mat{
  double** data;
  size_t col;
  size_t row;
};

struct Mat* MatCreate(size_t row, size_t col, double* data[]);
struct Mat* MatTranspose(struct Mat* mat); // create the transpose matrix of mat and return it
void MatDestroy(struct Mat* mat);
void MatPrint(struct Mat* mat);
struct Mat* MatAdd(struct Mat* mat_1, struct Mat* mat_2);// create a new matrix that adds mat_1 to mat_2 and return it
struct Mat* MatSub(struct Mat* mat_1, struct Mat* mat_2);// create a new matrix that substracts mat_1 with mat_2 and return it
struct Mat* MatHadamard(struct Mat* mat_1, struct Mat* mat_2);// create a new matrix by performing the Hadamard product between mat_1 and mat_2 and return it
struct Mat* MatMult(struct Mat* mat_1, struct Mat* mat_2);// perform the matrix multiplication between mat_1 and mat_2 and return the resulting matrix
struct Mat* MatFunc(struct Mat* mat, double (*f)(double));// create a new matrix by applying the function f on every element of mat_1 and return it
struct Mat* MatScalar(struct Mat* mat, double scalar); // create a new matrix and by multiplying each element with the given scalar
double Relu(double x);
double ReluPrime(double x);
