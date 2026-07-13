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
struct Mat* MatTranspose(struct Mat* mat);
void MatDestroy(struct Mat* mat);
void MatPrint(struct Mat* mat);
struct Mat* MatAdd(struct Mat* mat_1, struct Mat* mat_2);
struct Mat* MatSub(struct Mat* mat_1, struct Mat* mat_2);
struct Mat* MatHadamard(struct Mat* mat_1, struct Mat* mat_2);
struct Mat* MatMult(struct Mat* mat_1, struct Mat* mat_2);
struct Mat* MatFunc(struct Mat* mat_1, double (*f)(double));
double Relu(double x);
double ReluPrime(double x);
