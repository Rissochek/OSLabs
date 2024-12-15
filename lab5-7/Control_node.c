#include <czmq.h>
#include <unistd.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Node Node;
struct Node{
    int id;
    int parent_id;
    size_t childs_count;
    size_t childs_container_size;
    Node* childs;
};

typedef struct{
    Node head;
} Tree;

size_t check_command_length(char* input);
Node* find_node_by_id(Node* head_node, int target_id);
int* compare_arrays(int *array1, size_t size1, int *array2, size_t size2);
void add_child_to_node(Node* node, Node* child);

int main(){
    // Публикуем по 5555 порту
    zsock_t* publisher = zsock_new_pub("tcp://*:5555");
    zsock_t* subscriber = NULL;
    // vars init
    Node head = {-1, 0, 0, 0, NULL};
    Tree tree = {head};
    int created_nodes[100] = {0};
    size_t counter = 0;
    int create_node_id; // id узла для создания
    int parent_id_for_new_node; // id отца нового узла
    int flag = 0; // был ли первый ребенок;
    zpoller_t* poller = zpoller_new(subscriber);

    printf("Starting main function\n");
    printf("Publisher socket created\n");

    // Логика обработки команд: create, exec, pingall, sub.
    while (1){
        if (flag == 1) {
            // Проверяем наличие сообщений в неблокирующем режиме
            zsock_t* ready_sock = zpoller_wait(poller, 2000);
            if (ready_sock) {
                char* input_string = zstr_recv(subscriber);
                if (!input_string) {
                    perror("Failed to receive message");
                    exit(EXIT_FAILURE);
                }
                printf("Received message: %s\n", input_string);

                char to_print[100] = {0}; // Инициализируем массив нулями
                // Обработка входящих сообщений от вычислительных узлов.
                char nodes_command[100];
                size_t counter = 0;
                sscanf(input_string, "%s", nodes_command);
                printf("Command: %s\n", nodes_command);

                // если это ответ на exec
                if (strcmp(nodes_command, "for") == 0) {
                    // проходим пока не найдем O, потом кайфуем
                    while (input_string[counter] != 'O') {
                        counter++;
                    }
                    // Копируем все, что идет далее (включая 'O')
                    strcpy(to_print, &input_string[counter]);
                    printf("Response to exec command: %s\n", to_print);
                // } else if (strcmp(nodes_command, "pingall") == 0) {
                //     int alive_list[100] = {0};
                //     int alive_node_id;
                //     size_t local_counter = 0;
                //     size_t local_counter2 = 0;
                //     sscanf(input_string, "pingall %d", &alive_node_id);
                //     alive_list[local_counter] = alive_node_id;
                //     int* not_alive = compare_arrays(alive_list, local_counter, created_nodes, counter);
                //     printf("OK: ");
                //     while (not_alive[local_counter2] != 0) {
                //         printf("%d; ", not_alive[local_counter2++]);
                //     }
                //     printf("\n");
                }
                free(input_string);
            }
        }

        // считываем команду
        char command[100];
        scanf("%s ", command);
        printf("Received command: %s\n", command);

        // create (вроде бы фулл правильно написан (надеюсь))
        if (strcmp(command, "create") == 0){
            scanf("%d %d", &create_node_id, &parent_id_for_new_node); // считываем данные команды id и parent_id
            printf("create_node_id: %d, parent_id_for_new_node: %d\n", create_node_id, parent_id_for_new_node);

            // поиск родителя
            Node* parent_node = find_node_by_id(&tree.head, parent_id_for_new_node);
            if (parent_node == NULL){
                printf("Error: Parent not found\n");
            }
            // если родитель есть
            else{
                if (find_node_by_id(&tree.head, create_node_id) != NULL){
                    printf("Error: Already exists\n");
                }else{
                    // создание новой ноды в дереве
                    created_nodes[counter++] = create_node_id;
                    Node new_node = {create_node_id, parent_id_for_new_node, 0, 0, NULL};
                    add_child_to_node(parent_node, &new_node);
                    printf("New node created and added to parent\n");

                    // создание нового узла
                    pid_t child_proccess_id = fork();
                    if (child_proccess_id == -1){
                        perror("Failed to fork(unlucky moment)");
                        exit(EXIT_FAILURE);
                    }
                    // создаем дочерний процесс
                    else if (child_proccess_id == 0){
                        char create_node_id_str[20];
                        char parent_id_for_new_node_str[20];
                        snprintf(create_node_id_str, sizeof(create_node_id_str), "%d", create_node_id);
                        snprintf(parent_id_for_new_node_str, sizeof(parent_id_for_new_node_str), "%d", parent_id_for_new_node);
                        char* new_argv[] = {"./counting", create_node_id_str, parent_id_for_new_node_str, NULL};
                        if(execv(new_argv[0], new_argv) == -1){
                            perror("Failed to execv new process (unlucky moment)");
                            exit(EXIT_FAILURE);
                        }
                    }
                    // логика в главном процессе (по идее суб на дочерний процесс, если главный это его родитель)
                    else {
                        char address[100];
                        size_t port_to_sub;

                        if (flag == 0 && parent_id_for_new_node == -1){
                            // формируем адрес для подписки и подписываемся по нему (создаем сокет)
                            port_to_sub = 5555 + create_node_id;
                            snprintf(address, sizeof(address), "tcp://localhost:%zu", port_to_sub);
                            subscriber = zsock_new_sub(address, "");
                            zpoller_add(poller, subscriber);
                            flag = 1;
                            printf("Subscriber socket created and connected to %s\n", address);
                        }
                        else if (flag != 0 && parent_id_for_new_node == -1){
                            // формируем адрес для подписки и подписываемся по нему (просто подписываемся с готового сокета)
                            port_to_sub = 5555 + create_node_id;
                            snprintf(address, sizeof(address), "tcp://localhost:%zu", port_to_sub);
                            zsock_connect(subscriber, address);
                            printf("Subscriber connected to %s\n", address);
                        }
                        else if (parent_id_for_new_node != -1){
                            // логика отправки команды для подписки родительского процесса на дочерний (для обратной связи)
                            char message_to_sub[100];
                            snprintf(message_to_sub, sizeof(message_to_sub), "sub for %d on %d", parent_id_for_new_node, create_node_id);
                            zstr_send(publisher, message_to_sub);
                            printf("Sent sub command: %s\n", message_to_sub);
                        }
                    }
                }
            }
        }
        else if(strcmp(command, "exec") == 0){
            // считываем строку значений команды в формате node_id var_name var_value
            int target_node_id;
            char var_name[30];
            size_t var_value;
            char input[100];
            char command_message[100];
            if (fgets(input, sizeof(input), stdin) == NULL) {
                perror("Ошибка чтения ввода\n");
                return 1;
            }
            // убираем \n в конце строки
            input[strcspn(input, "\n")] = 0;
            size_t words_count = check_command_length(input);
            printf("Input: %s, words_count: %zu\n", input, words_count);

            if (words_count == 2){
                sscanf(input, "%d %s", &target_node_id, var_name);
                snprintf(command_message, sizeof(command_message), "exec for %d with %s %zu", target_node_id, var_name, words_count);
            }
            else if (words_count == 3){
                sscanf(input, "%d %s %zu", &target_node_id, var_name, &var_value);
                snprintf(command_message, sizeof(command_message), "exec for %d with %s %zu %zu", target_node_id, var_name, words_count, var_value);
            }

            printf("Sending command message to publisher: %s\n", command_message);
            zstr_send(publisher, command_message);
        }
        // pingall
        else if (strcmp(command, "pingall") == 0){
            char message[100];
            // отправляем родительский pingall
            snprintf(message, sizeof(message), "pingall %d", -1);
            zstr_send(publisher, message);
            printf("Sent pingall command: %s\n", message);
        }
        else if (strcmp(command, "kill") == 0){
            printf("Exiting program\n");
            exit(EXIT_SUCCESS);
        }
    }

    zpoller_destroy(&poller);
    return 0;
}

size_t check_command_length(char* input){
    char *str_copy = strdup(input);
    if (str_copy == NULL) {
        perror("Ошибка выделения памяти\n");
        return -1;
    }

    char *token;
    size_t word_count = 0;

    token = strtok(str_copy, " \t\n");
    while (token != NULL) {
        word_count++;
        token = strtok(NULL, " \t\n");
    }

    free(str_copy);
    return word_count;
}

Node* find_node_by_id(Node* head_node, int target_id){
    printf("Finding node by id: %d\n", target_id);
    if (head_node->id == target_id){
        printf("Found node: %p\n", (void*)head_node);
        return head_node;
    }else{
        for(size_t i = 0; i < head_node->childs_count; i++){
            printf("Checking child node: %d\n", head_node->childs[i].id);
            Node* result = find_node_by_id(&head_node->childs[i], target_id);
            if (result != NULL){
                return result;
            }
        }
    }
    printf("Node not found\n");
    return NULL;
}

void add_child_to_node(Node* node, Node* child){
    printf("Adding child to node\n");
    if (node->childs == NULL){
        node->childs = malloc(sizeof(Node) * 3);
        if (node->childs == NULL) {
            perror("Failed to allocate memory for childs");
            exit(EXIT_FAILURE);
        }
        node->childs_container_size = 3;
    }
    if (node->childs != NULL){
        if (node->childs_count == node->childs_container_size){
            printf("Container is full\n");
            Node* buffer = realloc(node->childs, node->childs_container_size * 2 * sizeof(Node));
            if (buffer == NULL){
                perror("Failed to reallocate memory for childs");
                exit(EXIT_FAILURE);
            }
            node->childs = buffer;
            node->childs_container_size = node->childs_container_size * 2;
        }
        node->childs[node->childs_count] = *child;
        printf("%d\n", node->childs[node->childs_count].id);
        node->childs_count++;
    }
    printf("Child added to node\n");
}

bool contains(int *array, size_t size, int element) {
    for (size_t i = 0; i < size; i++) {
        if (array[i] == element) {
            return true;
        }
    }
    return false;
}

int* compare_arrays(int *array1, size_t size1, int *array2, size_t size2) {
    int arr[100] = {0};
    size_t counter = 0;
    for (size_t i = 0; i < size2; i++) {
        if (!contains(array1, size1, array2[i])) {
            arr[counter++] = array2[i];
        }
    }
    return arr;
}
