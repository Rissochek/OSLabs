#include <stdlib.h>
#include <stdio.h>
int GCF(int a, int b) {
    printf("Function is evclid_gfc\n");
    int c;
    while (b) {
        c = a % b;
        a = b;
        b = c;        
    }
    return abs(a);
}

float Square(float a, float b){
    printf("Function is square_square\n");
    return a * b;
}
