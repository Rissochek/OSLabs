#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    size_t row; size_t row_start; size_t row_end; size_t columns_count; double** massive; double gen_value;
} GaussArgs;

void* func_for_threads(void* args){
    GaussArgs *gauss_args = (GaussArgs *)args;
    size_t row = gauss_args->row;
    size_t row_start = gauss_args->row_start;
    size_t row_end = gauss_args->row_end; 
    size_t columns_count = gauss_args->columns_count;
    double** massive = gauss_args->massive;
    double gen_value = gauss_args->gen_value;
    double to_replace = 0;
    for (size_t sub_row = row_start; sub_row < row_end; sub_row++){
        double value_under_gen = massive[sub_row][row];
        double coff = value_under_gen/gen_value;
        for (size_t col = row; col < columns_count; col++){
            to_replace = massive[sub_row][col] - massive[row][col]*coff;
            massive[sub_row][col] = to_replace;
        }
    }
    pthread_exit(0);
}

//massive[строки][колонны]  massive[row][column]
void Gauss_func(size_t columns_count, size_t rows_count, double** massive, size_t thread_count) {
    pthread_t tid[thread_count];
    size_t rows_per_thread = 1;
    size_t remaining_rows = 0;
    GaussArgs* args_array = malloc(thread_count * sizeof(GaussArgs));
    size_t pthread_used = 0;
    for (size_t row = 0; row < rows_count; row++) {
        double gen_value = massive[row][row];
        if (gen_value != 0.0) {
            if (row < rows_count - 1) {
                for (size_t i = 0; i < thread_count; i++) {
                    if (thread_count < (rows_count - 1) - row ){
                        rows_per_thread = ((rows_count- 1) - row) / thread_count;
                        remaining_rows = (rows_count - 1 - row) % thread_count;
                    }
                    size_t row_start = (i * rows_per_thread) + 1 + row;
                    size_t row_end = row_start + rows_per_thread;
                    if (i == thread_count - 1) {
                        row_end += remaining_rows;
                    }
                    if (row_end <= rows_count){
                        args_array[i] = (GaussArgs){row, row_start, row_end, columns_count, massive, gen_value};
                        pthread_create(&tid[i], NULL, func_for_threads, &args_array[i]);
                        pthread_used++;
                    }else{
                        break;
                    }
                }
                for (size_t i = 0; i < pthread_used; i++) {
                        pthread_join(tid[i], NULL);
                    }
                rows_per_thread = 1;
                remaining_rows = 0;
                pthread_used = 0;
                }
            }
        }
    free(args_array);
}

void generate_random_matrix(double** massive, size_t rows_count, size_t col_count) {
    srand(time(NULL));
    for (size_t row = 0; row < rows_count; row++) {
        for (size_t col = 0; col < col_count; col++) {
            massive[row][col] = (double)(rand() % 101); 
        }
    }
}

int main(int argc, char *argv[]){
    size_t rows_count = 3000;
    size_t col_count = 3001;
    double**massive = malloc(sizeof(double *) * rows_count);
    for (size_t line = 0; line < rows_count; line++){
        massive[line] = malloc(col_count * sizeof(double));
    }
    generate_random_matrix(massive, rows_count, col_count);
    const size_t thread_count = (size_t)atoi(argv[1]);
    clock_t start_time = clock();
    Gauss_func(col_count, rows_count, massive, thread_count);
    clock_t end_time = clock(); 
    double elapsed_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    printf("Время выполнения Gauss_func: %f секунд\n", elapsed_time);
    for (size_t i = 0; i < rows_count; i++){
        free(massive[i]);
    }
    free(massive);
}