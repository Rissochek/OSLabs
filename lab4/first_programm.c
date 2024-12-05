#include <stdio.h>
#include <stdlib.h>

extern int GCF(int a, int b);
extern float Square(float a, float b);

int main(int argc, char** argv){
    if (argc != 7){
        printf("Usage: %s 1 <arg1> <arg2> 2 <arg1> <arg2>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    printf("Evclid algorithm result: %d\n", GCF(atoi(argv[2]), atoi(argv[3])));
    printf("Stupid algorithm result: %f\n", Square(atof(argv[5]), atof(argv[6])));
}