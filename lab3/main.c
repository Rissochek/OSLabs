#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) 

{ 
    const char* back_name = "Lab3.back";
    unsigned perms = S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH;
    char  *input_data = malloc(sizeof(char) * 50);
    size_t   size = 50;
    size_t map_size = 0;
    pid_t  cpid;
    int    counter = 0;
    char ch = ' ';

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
        }
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

    int fd = shm_open(back_name, O_RDWR | O_CREAT, perms);
      if (fd == -1) {
        perror("OPEN");
        exit(EXIT_FAILURE);
    }

    map_size = size + (size_t)strlen(argv[1]) + (size_t)1;
    ftruncate(fd, (off_t)size);

    caddr_t memptr = mmap(
        NULL,
        map_size,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd,
        0);
    
    if (memptr == MAP_FAILED) {
        perror("MMAP");
        exit(EXIT_FAILURE);
    }
    memset(memptr, '\0', map_size);
    sprintf(memptr, "%s", argv[1]);
    sprintf(memptr + strlen(memptr), "%s", "|");
    sprintf(memptr + strlen(memptr), "%s", input_data);

    cpid = fork();
    if (cpid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (cpid == 0){
        munmap(memptr, size);
	    close(fd);
        execl("child", "child", NULL);
        perror("EXECL");
	    exit(EXIT_FAILURE);
    }else{
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