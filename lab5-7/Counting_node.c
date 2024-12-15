#include <czmq.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct{
    char* var_name;
    size_t var_value;
} CustomVar;

typedef struct{
    CustomVar* vars_container;
    size_t vars_count;
    size_t vars_container_size;
} VarContainer;

void add_to_var_container(VarContainer* vars_container, CustomVar* some_var);
bool check_var_is_existing(VarContainer* vars_container, CustomVar* var_to_check);
CustomVar find_var(VarContainer* vars_container, CustomVar* var_to_check);

int main(int argc, char* argv[]){
    int this_node_id = atoi(argv[1]);
    int parent_node_id = atoi(argv[2]);
    VarContainer var_container = {NULL, 0, 0};
    int target_node_id;
    char to_send[100];
    //формируем строку адреса слушания
    char address[100];
    size_t port_to_sub;
    size_t port_to_pub = 5555 + this_node_id;
    if (parent_node_id == -1){
        port_to_sub = 5555;
    }else{
        port_to_sub = 5555 + parent_node_id;
    }
    snprintf(address, sizeof(address), "tcp://localhost:%zu", port_to_sub);
    zsock_t* subscriber = zsock_new_sub(address, ""); //слушаем все, фильтровать будем потом
    snprintf(address, sizeof(address), "tcp://localhost:%zu", port_to_pub);
    zsock_t* publisher = zsock_new_pub(address);

    printf("CHILD INFO\n");
    printf("Node ID: %d, Parent Node ID: %d\n", this_node_id, parent_node_id);
    printf("Subscriber address: %s\n", address);
    printf("Publisher address: %s\n", address);

    //обработка команд exec, sub, pingall, OK
    while (1){
        //считываем сообщение от издателя
        
        char* input_string = zstr_recv(subscriber);
        if (!input_string) {
            perror("Failed to receive message");
            exit(EXIT_FAILURE);
        }
        printf("Received message: %s\n", input_string);

        char command[100];
        int command_node_id;
        //ВАЖНО после sscanf каретка не передвигается по строке!!!!
        sscanf(input_string, "%s", command);
        printf("Command: %s\n", command);

        //обработка команды sub
        if (strcmp(command, "sub") == 0){
            int node_id_to_sub;
            sscanf(input_string, "sub for %d on %d", &command_node_id, &node_id_to_sub);
            printf("Command Node ID: %d, Node ID to Sub: %d\n", command_node_id, node_id_to_sub);
            //если команда для нас, то подписываемся на нашего нового ребенка
            if (command_node_id == this_node_id){
                port_to_sub = 5555 + node_id_to_sub;
                snprintf(address, sizeof(address), "tcp://localhost:%zu", port_to_sub);
                zsock_connect(subscriber, address);
                printf("Subscribed to new child node at address: %s\n", address);
            }
            //если команда не для нас, то отсылаем нашим детям
            else{
                zstr_send(publisher, input_string);
                printf("Forwarding sub command to children: %s\n", input_string);
            }
        }
        else if (strcmp(command, "exec") == 0){
            //"exec for %zu with %s %zu", target_node_id, var_name, words_count
            //or "exec for %zu with %s %zu %zu", target_node_id, var_name, words_count, var_value
            sscanf(input_string, "exec for %d", &target_node_id);
            printf("Target Node ID: %d\n", target_node_id);
            //если мы не таргет нода, то отсылаем подписчикам!!!!
            if (target_node_id != this_node_id){
                zstr_send(publisher, input_string);
                printf("Forwarding exec command to children: %s\n", input_string);
            }
            else{
                //логика обработки exec, если мы таргет нода
                char var_name[100];
                size_t words_count;
                size_t var_value;
                sscanf(input_string, "exec for %d with %s %zu", &target_node_id, var_name, &words_count);
                CustomVar curr_var;
                bool is_exists;
                printf("Words count: %zu\n", words_count);
                if (words_count == 2){
                    curr_var = (CustomVar){strdup(var_name), 0};
                    if (check_var_is_existing(&var_container, &curr_var)){
                        curr_var = find_var(&var_container, &curr_var);
                        snprintf(to_send, sizeof(to_send), "for %d f1 OK:%d:%zu", parent_node_id, this_node_id, curr_var.var_value);
                        zstr_send(publisher, to_send);
                        printf("Sending response for exec command: %s\n", to_send);
                    }
                    else{
                        snprintf(to_send, sizeof(to_send), "for %d f2 OK:%d: %s Not found", parent_node_id, this_node_id, curr_var.var_name);
                        zstr_send(publisher, to_send);
                        printf("Sending response for exec command: %s\n", to_send);
                    }
                    free(curr_var.var_name);
                }
                else if(words_count == 3){
                    sscanf(input_string, "exec for %d with %s %zu %zu", &target_node_id, var_name, &words_count, &var_value);
                    curr_var = (CustomVar){strdup(var_name), var_value};
                    add_to_var_container(&var_container, &curr_var);
                    snprintf(to_send, sizeof(to_send), "for %d f3 OK:%d", parent_node_id, this_node_id);
                    zstr_send(publisher, to_send);
                    printf("Sending response for exec command: %s\n", to_send);
                    free(curr_var.var_name);
                }
            }
        }
        else if(strcmp(command, "pingall") == 0){
            //сначала отправляем сообщение детям и через себя передаем наверх по дереву
            int parent_ping_id;
            sscanf(input_string, "pingall %d", &parent_ping_id);
            printf("Parent Ping ID: %d\n", parent_ping_id);
            if (parent_ping_id == parent_node_id){
                char message[100];
                //отправляем родительский pingall
                snprintf(message, sizeof(message), "pingall %d", this_node_id);
                zstr_send(publisher, message);
                printf("Sending parent pingall: %s\n", message);
            }
            else if(parent_ping_id != this_node_id){
                char message[100];
                //отправляем дочерний pingall
                snprintf(message, sizeof(message), "pingall %d", parent_ping_id);
                zstr_send(publisher, message);
                printf("Sending child pingall: %s\n", message);
            }
        }
        //логика обработки входящего сообщения обработки команды exec
        else if (strcmp(command, "for") == 0){
            char format[10];
            char message[100];
            int local_parent_node_id;
            int local_this_node_id;
            char var_name[40];
            int var_value;
            sscanf(input_string, "for %d %s", &target_node_id, format);
            printf("Format: %s\n", format);
            //если адресовано нам то обрабатываем, а если нет, то забиваем (чтобы не было бесконечной отправки тудым сюдым)
            if (target_node_id == this_node_id){
                //format1 --- "for %zu f1 OK:%zu:%zu", parent_node_id, this_node_id, curr_var.var_value
                if (strcmp(format, "f1") == 0){
                    sscanf(input_string, "for %d f1 OK:%d:%d", &local_parent_node_id, &local_this_node_id, &var_value);
                    snprintf(message, sizeof(message), "for %d f1 OK:%d:%d", parent_node_id, local_this_node_id, var_value);
                }
                //format2 --- "for %zu f2 OK:%zu: %s Not found", parent_node_id, this_node_id, curr_var.var_name
                else if (strcmp(format, "f2") == 0){
                    sscanf(input_string, "for %d f2 OK:%d: %s Not found", &local_parent_node_id, &local_this_node_id, var_name);
                    snprintf(message, sizeof(message), "for %d f2 OK:%d: %s Not found", parent_node_id, local_this_node_id, var_name);
                }
                //format3 --- "for %zu f3 OK:%zu", parent_node_id, this_node_id
                else if (strcmp(format, "f3") == 0){
                    sscanf(input_string, "for %d f3 OK:%d", &local_parent_node_id, &local_this_node_id);
                    snprintf(message, sizeof(message), "for %d f3 OK:%d", parent_node_id, local_this_node_id);
                }
                zstr_send(publisher, message);
                printf("Sending response for exec command: %s\n", message);
            }
        }
        free(input_string);
    }
}

void add_to_var_container(VarContainer* vars_container, CustomVar* some_var){
    printf("Adding variable to container: %s, %zu\n", some_var->var_name, some_var->var_value);
    if (vars_container->vars_count == 0 && vars_container->vars_container == NULL){
        vars_container->vars_container = malloc(sizeof(CustomVar) * 3);
        vars_container->vars_container_size = 3;
    }
    if (vars_container->vars_count == vars_container->vars_container_size){
        CustomVar* buffer = realloc(vars_container->vars_container, vars_container->vars_container_size * 2 * sizeof(CustomVar));
        if (buffer == NULL){
            perror("Ошибка выделения памяти\n");
            exit(EXIT_FAILURE);
        }
        vars_container->vars_container = buffer;
        vars_container->vars_container_size = vars_container->vars_container_size * 2;
        vars_container->vars_container[vars_container->vars_count] = *some_var;
        vars_container->vars_count++;
    }
    else if (vars_container->vars_count < vars_container->vars_container_size){
        vars_container->vars_container[vars_container->vars_count] = *some_var;
        printf("VarName: %s\n", vars_container->vars_container[vars_container->vars_count].var_name);
        vars_container->vars_count++;
    }
    printf("Variable added to container: %s, %zu\n", some_var->var_name, some_var->var_value);
}

bool check_var_is_existing(VarContainer* vars_container, CustomVar* var_to_check){
    printf("Checking if variable exists: %s\n", var_to_check->var_name);
    CustomVar* vars_container_local = vars_container->vars_container;
    for (size_t i = 0; i < vars_container->vars_count; i++){
        if (strcmp(vars_container_local[i].var_name, var_to_check->var_name) == 0){
            printf("Variable exists: %s\n", var_to_check->var_name);
            return true;
        }
    }
    printf("Variable does not exist: %s\n", var_to_check->var_name);
    return false;
}

CustomVar find_var(VarContainer* vars_container, CustomVar* var_to_check){
    printf("Finding variable: %s\n", var_to_check->var_name);
    CustomVar* vars_container_local = vars_container->vars_container;
    for (size_t i = 0; i < vars_container->vars_count; i++){
        if (strcmp(vars_container_local[i].var_name, var_to_check->var_name) == 0){
            printf("Variable found: %s, %zu\n", var_to_check->var_name, vars_container_local[i].var_value);
            return vars_container_local[i];
        }
    }
    printf("Variable not found: %s\n", var_to_check->var_name);
    return (CustomVar){NULL, 0};
}
