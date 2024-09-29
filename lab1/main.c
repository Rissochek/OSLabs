#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>

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

        dup2(pipefd_from_parent_to_child[0], STDIN_FILENO);
        dup2(pipefd_from_child_to_parent[1], STDOUT_FILENO);

        close(pipefd_from_parent_to_child[0]);
        close(pipefd_from_child_to_parent[1]); 
        char *args[] = {"./child", NULL};
        if (execv(args[0], args) == -1) {
            perror("execv");
            exit(EXIT_FAILURE);
        }
        close(pipefd_from_parent_to_child[0]);          
        close(pipefd_from_child_to_parent[1]); 
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
