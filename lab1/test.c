#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

float calc_func(char* data, int size);

int main(){
    char* data = malloc(sizeof(char) * 8);
    data[0] = '2';
    data[1] = ' ';
    data[2] = '2';
    data[3] = '\n';
    int size = 8;
    calc_func(data, size);
    free(data);
    return 0;
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
    for (int i = 0; i < size; i++){
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