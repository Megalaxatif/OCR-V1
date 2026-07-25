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
struct Mat* MatAddInternal(struct Mat* mat_1, struct Mat* mat_2); // add mat_2 to mat_1 and return mat_1
struct Mat* MatSub(struct Mat* mat_1, struct Mat* mat_2);// create a new matrix that substracts mat_1 with mat_2 and return it
struct Mat* MatSubInternal(struct Mat* mat_1, struct Mat* mat_2);// substract mat_2 to mat_1 and return mat_1
struct Mat* MatHadamard(struct Mat* mat_1, struct Mat* mat_2);// create a new matrix by performing the Hadamard product between mat_1 and mat_2 and return it
struct Mat* MatHadamardInternal(struct Mat* mat_1, struct Mat* mat_2);// perform the Hadamard product between mat_1 and mat_2 and return mat_1
struct Mat* MatMult(struct Mat* mat_1, struct Mat* mat_2);// perform the matrix multiplication between mat_1 and mat_2 and return the resulting matrix
struct Mat* MatFunc(struct Mat* mat, double (*f)(double));// create a new matrix by applying the function f on every element of mat and return it
struct Mat* MatFuncInternal(struct Mat* mat, double (*f)(double)); // apply the function f on every element of mat and return mat
struct Mat* MatScalar(struct Mat* mat, double scalar); // create a new matrix and by multiplying each element with the given scalar
struct Mat* MatScalarInternal(struct Mat* mat, double scalar); // multiply each element of mat by the scalar and return mat
double Relu(double x);
double ReluPrime(double x);
size_t Random(size_t min, size_t max); // generate a random number between min and max and return it
