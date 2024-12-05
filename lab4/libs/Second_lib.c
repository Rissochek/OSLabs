#include <stdio.h>
#include <stdlib.h>

int GCF(int a, int b){
    printf("Function is stupid_gfc\n");
    int minimal = 0;
    if (abs(a) < abs(b)){
        minimal = abs(a);
    }else{
        minimal = abs(b);
    }
    for (int i = minimal; i > 0; i--){
        if (a % i == 0 && b % i == 0){
            return i;
        }
    }
}

float Square(float a, float b){
    printf("Function is square_triangle\n");
    return 0.5*a*b;
}

