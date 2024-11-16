#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

bool check_on_zeros(char* data, int size);
float calc_func(char* data, int size);

int main(){
    const char* back_name = "Lab3.back";
    unsigned perms = S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH;
    size_t counter = 0;
    char filename[20];
    size_t readed_data_id = 0;
    int map_fd = shm_open(back_name, O_RDWR, perms);
    if (map_fd < 0) {
        perror("SHM_OPEN");
        exit(EXIT_FAILURE);
    }

    struct stat statbuf;
    fstat(map_fd, &statbuf);
    const size_t map_size = statbuf.st_size;

    caddr_t memptr = mmap(
        NULL,
        map_size,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        map_fd,
        0);
    if (memptr == MAP_FAILED) {
        perror("MMAP");
        exit(EXIT_FAILURE);
    }

    char  *input_data = malloc(sizeof(char) * (map_size + 1));

    for (size_t i = 0; i < map_size; i++){
        if (memptr[i] != '|'){
            filename[i] = memptr[i];
            } else{
                filename[i] = '\0';
                readed_data_id = i+1;
                break;
            }
    }
    for (size_t i = readed_data_id; i < map_size; i++) {
        input_data[i - readed_data_id] = memptr[i];
    }
    float tmp = 0;
    if (check_on_zeros(input_data, map_size) == 0){
        tmp = calc_func(input_data, map_size);
    } else{
        perror("Cannot devide by zero\n");
        free(input_data);
        _exit(EXIT_FAILURE);
    }

    FILE *fptr;
    fptr = fopen(filename, "w");
    fprintf(fptr, "%f", tmp);
    fclose(fptr);
    free(input_data);
    munmap(memptr, map_size);
    close(map_fd);
    _exit(EXIT_SUCCESS);
}

bool check_on_zeros(char* data, int size){
    for (int i = 1; i < size; i++){
        if (data[i] == '0'){
            if (data[i+1] == ' ' || data[i+1] == '\0' || data[i+1] == '\n'){
                if (data[i-1] == ' '){
                    return true;
                }
            }
            if (data[i+1] == '.' || data[i+1] == ','){
                int non_zeros_counter = 0;
                i += 2;
                while(data[i] != ' ' && data[i] != '\0'){
                    char sth = data[i++];
                    if (sth != '0'){
                        if (sth != '\0'){
                            if (sth != '\n'){
                                non_zeros_counter += 1;
                            }
                        }
                    }
                }
                if (non_zeros_counter == 0){
                    return true;
                }
                non_zeros_counter = 0;
            }
        }
    }
    return false;
}

float calc_func(char* data, int size){
    int to_alloc = 100;
    char *buffer = malloc(sizeof(char) * to_alloc);
    int j = 0;
    char ch;
    float first_number;
    bool flag = false;
    float number = 1;
    float tmp = 0;
    fflush(stdout);
    for (int i = 0; i < size; i++){
    fflush(stdout);
        if (data[i] != '\0'){
            while ((ch = data[i]) != ' ' && ch != '\n' && ch != '\0') {
                buffer[j] = ch;
                j++;
                i++;
                if (i >= size){
                    break;
                }
            }
            if (flag == false){
                sscanf(buffer, "%f", &first_number);
                number = first_number;
                flag = true;
            }else{
                sscanf(buffer, "%f", &tmp);
                number = number / tmp;
            }
            j = 0;
            while (buffer[j] != '\0'){
                buffer[j++] = ' ';
                if (j >= to_alloc){
                    break;
                }
            }
            j = 0;
        }else{
            break;
        }
    }
    free(buffer);
    return number;
}