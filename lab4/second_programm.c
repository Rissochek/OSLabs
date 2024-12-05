#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

typedef enum{
  first_realization,
  second_realization,
} contract;

contract cont = first_realization;

const char* lib_name_first = "./libs/First_lib.so";
const char* lib_name_second = "./libs/Second_lib.so";

float (*Square)(float, float) = NULL;
int (*GCF)(int, int) = NULL;

void *lib_chosen = NULL;

void load_libs(contract contract){
    const char* name;
    switch (contract) {
        case first_realization:{
            name = lib_name_first;
            break;
        }
        case second_realization:{
            name = lib_name_second;
            break;
        }
    }
    lib_chosen = dlopen(name, RTLD_LAZY);
    if (lib_chosen == NULL){
        perror("dlopen");
        exit(EXIT_FAILURE);
    }
}

void load_contract(){
  load_libs(cont);
  Square = dlsym(lib_chosen, "Square");
  GCF = dlsym(lib_chosen, "GCF");
}

void change_contract(){
  dlclose(lib_chosen);
  switch (cont) {
	case first_realization:{
		cont = second_realization;
		break;
	}
	case second_realization:{
	  cont = first_realization;
	  break;
	}
  }
  load_contract();
}

int main(){
    load_contract();
    int value = 0;
    while (scanf("%d", &value) != EOF){
        switch (value) {
            case 0:{
                change_contract();
                printf("contract has been changed\n");
                switch (cont) {
                case first_realization: {
                    printf("contract is first_realization\n");
                    break;
                }
                case second_realization:{
                    printf("contract is second_realization\n");
                }
            }
            break;
            }
            case 1:{
                int a, b;
                if (scanf("%d %d", &a, &b) == 2){
                    printf("%d\n", GCF(a, b));
                }
                break;
            }
            case 2:{
                float a, b;
                if (scanf("%f %f", &a, &b) == 2){
                    printf("%f\n", Square(a,b));   
                }
            }
        }
    }
    dlclose(lib_chosen);
    return 0;
}