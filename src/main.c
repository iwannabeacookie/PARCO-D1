#include <omp.h>
#include "../include/init_matrix.h"
#include "../include/sequential.h"
#include "../include/parallel.h"
#include "../include/utils.h"

#define MATRIX_DIMENSION 4096

int main(int argc, char *argv[])
{
    float** p = init_matrix_sequential(MATRIX_DIMENSION);
    float** m = init_matrix_parallel(MATRIX_DIMENSION);

    // print_matrix(m, MATRIX_DIMENSION);

    is_symmetric(m, MATRIX_DIMENSION);

    is_symmetric_implicit(m, MATRIX_DIMENSION);

    is_symmetric_omp(m, MATRIX_DIMENSION);

    float ** t = transpose(m, MATRIX_DIMENSION);
    // print_matrix(t, MATRIX_DIMENSION);

    float ** i_t = transpose_implicit(m, MATRIX_DIMENSION);
    // print_matrix(i_t, MATRIX_DIMENSION);
    // correct_transpose(t, i_t, MATRIX_DIMENSION);

    float ** o_t = transpose_omp(m, MATRIX_DIMENSION);
    // print_matrix(o_t, MATRIX_DIMENSION);
    // correct_transpose(t, o_t, MATRIX_DIMENSION);

    float ** b_t = transpose_omp_block_based(m, MATRIX_DIMENSION);
    // print_matrix(b_t, MATRIX_DIMENSION);
    // correct_transpose(t, b_t, MATRIX_DIMENSION);

    return 0;
}
