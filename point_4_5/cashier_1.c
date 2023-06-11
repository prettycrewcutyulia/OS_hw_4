#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void sigfunc(int sig) {
    if (sig != SIGINT && sig != SIGTERM) {
        return;
    }

    printf("Sig finished\n");
    exit(10);
}

int main(int argc, char const *argv[]) {
    unsigned short server_port;
    const char *server_ip;

    int sock = 0;
    struct sockaddr_in serv_addr;
    int server_answer_1 = 0;
    int server_answer_2 = 0;

    // Получаем значения port и server IP с помощью аргументов командной строки
    if (argc != 3) {
        printf("Args: <port> <SERVER_IP>\n");
        return -1;
    }

    server_port = atoi(argv[1]);
    server_ip = argv[2];

    // Создаем UDP сокет
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);

    // Устанавливаем адрес сервера
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    int identificator = 2;
    sendto(sock, &identificator, sizeof(int), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    sleep(1);
    int cashier_id = 1;
    sendto(sock, &cashier_id, sizeof(int), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    sleep(1);
    printf("Cashier №%d is ready for work.\n", cashier_id);

    int num_customers;
    socklen_t server_addr_len = sizeof(serv_addr);
    recvfrom(sock, &num_customers, sizeof(num_customers), 0, (struct sockaddr *)&serv_addr, &server_addr_len);
    sleep(1);
    printf("Cashier №%d: received number of customers: %d\n", cashier_id, num_customers);

    bool server_is_connected = true;

    while (server_is_connected) {
        server_answer_1 = 0;
        server_answer_2 = 0;
        printf("Cashier №%d on work.\n", cashier_id);

        for (int i = 0; i < num_customers; ++i) {
            server_answer_1 = 0;
            // Отправляем значения index на сервер
            int index = 1;
            printf("Cashier №%d is ready for work.\n", cashier_id);

            sendto(sock, &index, sizeof(int), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
            sleep(1);

            recvfrom(sock, &server_answer_1, sizeof(server_answer_1), 0, (struct sockaddr *)&serv_addr, &server_addr_len);
            if (server_answer_1 != -1) {
                printf("Cashier №%d: served customer with id №%d\n", cashier_id, server_answer_1);
            } else {
                server_is_connected = false;
                break;
            }
            sleep(1);
        }

        // Получаем ответ от сервера
        recvfrom(sock, &server_answer_2, sizeof(server_answer_2), 0, (struct sockaddr *)&serv_addr, &server_addr_len);
        printf("Cashier №%d has ended his work. Cashier served %d customer\n", cashier_id, server_answer_2);
        server_is_connected = false;
    }
    return 0;
}
