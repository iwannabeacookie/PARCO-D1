#include "../include/config.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

bool is_symmetric_omp(float **matrix, int n, long double* time) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    bool is_symmetric = true;

    #pragma omp parallel shared(is_symmetric) 
    {
        #pragma omp for reduction(&&:is_symmetric)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < i; j++) {
                if (matrix[i][j] != matrix[j][i]) {
                    #pragma omp atomic write
                    is_symmetric = false;
                    #pragma omp cancel for
                }
                #pragma omp cancellation point for
            }
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    *time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    if (get_config()->VERBOSE_LEVEL > 1) {
        printf("Computed that the matrix is %ssymmetric using OMP in: %Lf\n", is_symmetric ? "" : "not ", *time);
    }

    return is_symmetric;
}

float** transpose_omp(float **matrix, int n, long double* time) {
    struct timespec start, end;

    float **result = malloc(n * sizeof(float*));

    #pragma omp parallel
    {
        #pragma omp for schedule(dynamic)
        for (int i = 0; i < n; i++) {
            result[i] = malloc(n * sizeof(float));
        }

        #pragma omp single
        {
            clock_gettime(CLOCK_MONOTONIC, &start);
        }

        #pragma omp for collapse(2) schedule(guided)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                result[i][j] = matrix[j][i];
            }
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    *time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    if (get_config()->VERBOSE_LEVEL > 1) {
        printf("Computed the transpose using OMP in: %Lf\n", *time);
    }

    return result;
}

float** transpose_omp_block_based(float **matrix, int n, int block_size, long double* time) {
    struct timespec start, end;

    float **result = malloc(n * sizeof(float*));
    for (int i = 0; i < n; i++) {
        result[i] = malloc(n * sizeof(float));
    }

    #pragma omp parallel
    {
        #pragma omp single
        {
            clock_gettime(CLOCK_MONOTONIC, &start);
        }

        #pragma omp for collapse(2) schedule(dynamic)
        for (int i = 0; i < n; i += block_size) {
            for (int j = 0; j < n; j += block_size) {
                for (int ii = i; ii < i + block_size && ii < n; ii++) {
                    for (int jj = j; jj < j + block_size && jj < n; jj++) {
                        result[ii][jj] = matrix[jj][ii];
                    }
                }
            }
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    *time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    if (get_config()->VERBOSE_LEVEL > 1) {
        printf("Computed the block-based (size %d) transpose using OMP in: %Lf\n", block_size, *time);
    }

    return result;
}

// Tile distributed implementation disbributes the tile operations between threads scheduled statically to match the cache line size
float ** transpose_omp_tile_distributed(float **matrix, int n, int tile_size, long double* time) {
    struct timespec start, end;

    float **result = malloc(n * sizeof(float*));
    for (int i = 0; i < n; i++) {
        result[i] = malloc(n * sizeof(float));
    }

    int i, j, ii, jj;

    #pragma omp parallel
    {
        #pragma omp single
        {
            clock_gettime(CLOCK_MONOTONIC, &start);
        }

        #pragma omp for private(i,j,ii,jj) schedule(static)
        for (i = 0; i < n; i += tile_size) {
            for (j = 0; j < n; j += tile_size) {
                for (ii = i; ii < i + tile_size && ii < n; ii++) {
                    for (jj = j; jj < j + tile_size && jj < n; jj++) {
                        result[ii][jj] = matrix[jj][ii];
                    }
                }
            }
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    *time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    if (get_config()->VERBOSE_LEVEL > 1) {
        printf("Computed the tile-based (size %d) transpose using OMP in: %Lf\n", tile_size, *time);
    }

    return result;
}

float ** transpose_omp_tasks(float **matrix, int n, int tile_size, long double* time) {
    struct timespec start, end;

    float **result = malloc(n * sizeof(float*));
    for (int i = 0; i < n; i++) {
        result[i] = malloc(n * sizeof(float));
    }

    int i, j;

    #pragma omp parallel
    {
        #pragma omp single nowait
        {
            clock_gettime(CLOCK_MONOTONIC, &start);

            for (i = 0; i < n; i += tile_size) {
                for (j = 0; j < n; j += tile_size) {
                    #pragma omp task firstprivate(i, j) shared(matrix, result)
                    {
                        int ii, jj;
                        int max_ii = (i + tile_size > n) ? n : i + tile_size;
                        int max_jj = (j + tile_size > n) ? n : j + tile_size;

                        for (ii = i; ii < max_ii; ii++) {
                            for (jj = j; jj < max_jj; jj++) {
                                result[ii][jj] = matrix[jj][ii];
                            }
                        }
                    }
                }
            }
        }

        #pragma omp taskwait
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    *time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    if (get_config()->VERBOSE_LEVEL > 1) {
        printf("Computed the transpose using OMP tasks in: %Lf\n", *time);
    }

    return result;
}
