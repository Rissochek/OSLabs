#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

bool check_on_zeros(char* data, int size);
float calc_func(char* data, int size);

int main(int argc, char *argv[]){
    char  *input_data = malloc(sizeof(char) * 50);
    char buf;
    char   filename[20];
    int    counter = 0;
    size_t   size = 50;
    int i = 0;
    char ch;
    while (read(STDIN_FILENO, &buf, 1) > 0){

            if (buf != '|'){
            filename[i++] = buf;
            } else{
                break;
            }
    }
    while (read(STDIN_FILENO, &buf, 1) > 0){
        if (counter < size){
                input_data[counter++] = buf;
            } else{
                size *= 2;
                char *buffer = realloc(input_data, size);
                if (buffer == NULL) {
                    perror("realloc failed");
                    free(input_data); 
                    _exit(EXIT_FAILURE);
                }else{
                    input_data = buffer;
                    input_data[counter++] = buf;
                }
            }
    }
    input_data[counter] = '\0';
    float tmp;
    if (check_on_zeros(input_data, size) == 0){
        tmp = calc_func(input_data, size);
    } else{
        perror("Cannot devide by zero\n");
        free(input_data);
        _exit(EXIT_FAILURE);
    }

    filename[i] = '\0';
    FILE *fptr;
    fptr = fopen(filename, "w");
    fprintf(fptr, "%f", tmp);
    fclose(fptr);
    free(input_data);
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
    char *buffer = malloc(sizeof(char) * 100);
    int j = 0;
    char ch;
    float first_number;
    bool flag = false;
    float number = 1;
    float tmp = 0;
    for (int i = 0; i < size; i++){
        if (data[i] != '\0'){
            while ((ch = data[i]) != ' ' && ch != '\n' && ch != '\0') {
                buffer[j++] = ch;
                i++;
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
            }
            j = 0;
        }else{
            break;
        }
    }
    free(buffer);
    return number;
}
