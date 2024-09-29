#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>

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

int main(int argc, char *argv[])
{
    int    pipefd_from_parent_to_child[2];
    int    pipefd_from_child_to_parent[2];
    char   buf;
    char  *input_data = malloc(sizeof(char) * 50);
    char   filename[20];
    size_t   size = 50;
    int    counter = 0;
    pid_t  cpid;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (pipe(pipefd_from_parent_to_child) == -1) {
        perror("pipe1");
        exit(EXIT_FAILURE);
    }

    if (pipe(pipefd_from_child_to_parent) == -1) {
        perror("pipe2");
        exit(EXIT_FAILURE);
    }

    cpid = fork();
    if (cpid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (cpid == 0) {    
        close(pipefd_from_parent_to_child[1]);          
        close(pipefd_from_child_to_parent[0]); 
        int i = 0;
        char ch;
        while (read(pipefd_from_parent_to_child[0], &buf, 1) > 0){
                if (buf != '|'){
                filename[i++] = buf;
                } else{
                    break;
                }
        }
        while (read(pipefd_from_parent_to_child[0], &buf, 1) > 0){
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
                        input_data[counter++] = ch;
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
        

        close(pipefd_from_parent_to_child[0]);
        close(pipefd_from_child_to_parent[1]);
        free(input_data);
        _exit(EXIT_SUCCESS);

    } else {            
        close(pipefd_from_parent_to_child[0]);         
        close(pipefd_from_child_to_parent[1]);
        char ch;
        while ((ch = getchar()) != EOF && ch != '\n'){
            if (counter < size){
                input_data[counter++] = ch;
            } else{
                size *= 2;
                char *buffer = realloc(input_data, size);
                if (buffer == NULL) {
                    perror("realloc failed");
                    exit(EXIT_FAILURE);
                }else{
                    input_data = buffer;
                    input_data[counter++] = ch;
                }
            }
        }
        input_data[counter] = '\0';

        write(pipefd_from_parent_to_child[1], argv[1], strlen(argv[1]));
        write(pipefd_from_parent_to_child[1], "|", 1);
        write(pipefd_from_parent_to_child[1], input_data, strlen(input_data));

        close(pipefd_from_parent_to_child[1]);          
        close(pipefd_from_child_to_parent[0]);
        int status = 0;
        wait(&status);                
        free(input_data);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            exit(EXIT_SUCCESS);
        } else {
            exit(EXIT_FAILURE);
        }
    }
}