#include "header/math.h"

int main(){

    double r1[] = {1, 2, 3};
    double r2[] = {0, 2, 1};
    double r3[] = {3, 1, 2};

    double* v1[] =
        {
            r1,
            r2,
            r3
        };

    double r4[] = {2, 1};
    double r5[] = {2, 2};
    double r6[] = {3, 3};

    double* v2[] =
        {
            r4,
            r5,
            r6
        };

    struct Mat* mat_1 = MatCreate(3, 3,  v1);
    printf("mat1:\n");
    MatPrint(mat_1);

    struct Mat* mat_2 = MatCreate(3, 2, v2);
    printf("mat2:\n");
    MatPrint(mat_2);

    struct Mat* mat_3 = MatTranspose(mat_2);
    printf("mat3:\n");
    MatPrint(mat_3);

    MatDestroy(mat_1);
    MatDestroy(mat_2);
    MatDestroy(mat_3);

    return 0;
}
