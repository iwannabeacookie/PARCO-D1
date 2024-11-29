#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    int MATRIX_DIMENSION;
    int BLOCK_SIZE;
    int NUM_RUNS;
    int BENCHMARK_SEQUENTIAL;
    int BENCHMARK_IMPLICIT;
    int BENCHMARK_OMP;
    int OMP_THREADS;
} Config;

// Function to initialize the configuration
void init_config(int argc, char *argv[]);

// Function to retrieve the configuration
const Config* get_config();

#endif // !CONFIG_H
