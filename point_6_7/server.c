#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#define BUFFER 1024


const int max_customer = 20;
void sendToObserver(const char *message);


// Состояние покупателя
typedef enum {
    FIRST = 1, // к 1 кассиру
    SECOND = 2 // ко 2 кассиру
} CustomerState;

typedef struct {
    CustomerState state;
    int id;
} Customer;

typedef struct {
    Customer customer[max_customer];
    int num_customer;
    bool is_start;
    int num_customer_cashier_1;
    int num_customer_cashier_2;
    int customer_cashier_1[max_customer];
    int customer_cashier_2[max_customer];
} Store;

Store *store;

typedef struct {
    int sockfd;
    struct sockaddr_in client_addr;
} CashierInfo;

CashierInfo cashiers[2]; // Информация о клиентах-кассирах

typedef struct {
    int sockfd;
    struct sockaddr_in client_addr;
} CustomerInfo;

CustomerInfo customer; // Информация о подключенном клиенте-покупателе

typedef struct {
    int sockfd;
    struct sockaddr_in client_addr;
} ObserverInfo;

ObserverInfo observer; // Информация о подключенном клиенте-наблюдателе

void sigfunc(int sig) {
    if (sig != SIGINT && sig != SIGTERM) {
        return;
    }

    printf("Sig finished\n");
    exit(10);
}

// Функция потока для обработки клиентского подключения
void *cashierHandler(void *cashier_info) {
    int cashier_id;
    int is_ready;
    int answer;
    int countOfServedCustomer = 0;

    CashierInfo *info = (CashierInfo *)cashier_info;
    int sockfd = info->sockfd;
    struct sockaddr_in client_addr = info->client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    printf("Cashier handler started\n");
    recvfrom(sockfd, &cashier_id, sizeof(int), 0, (struct sockaddr *) &client_addr, &client_addr_len);
    char buffer2[BUFFER];
    sprintf(buffer2,"Cashier has id%d.\n", cashier_id);
    sendToObserver(buffer2);
    printf("Cashier has id %d.\n", cashier_id);

    if (cashier_id == 1) {
        sendto(sockfd, &store->num_customer_cashier_1, sizeof(int), 0, (struct sockaddr *) &client_addr, client_addr_len);
        while (store->is_start) {
            sleep(1);
            char buffer1[BUFFER];
            sprintf(buffer1,"Cashier №%d is ready for work.\n", cashier_id);
            sendToObserver(buffer1);
            printf("Cashier №%d is ready for work.\n", cashier_id);
            for (int i = 0; i < store->num_customer_cashier_1; i++) {
                recvfrom(sockfd, &is_ready, sizeof(int), 0, (struct sockaddr *) &client_addr, &client_addr_len);
                answer = 0;
                if (is_ready == 1) { // Добавленная проверка is_ready
                    if (store->customer[store->customer_cashier_1[i]].state == FIRST && cashier_id == 1) {
                        // отправляем клиенту id покупателя;
                        answer = store->customer[store->customer_cashier_1[i]].id;
                        countOfServedCustomer++;
                        char buffer[BUFFER];
                        sprintf(buffer,"Cashier №%d is served customer №%d\n", cashier_id, answer);
                        sendToObserver(buffer);
                        printf("Cashier №%d is serving customer №%d\n", cashier_id, answer);
                        sendto(sockfd, &answer, sizeof(int), 0, (struct sockaddr *) &client_addr, client_addr_len);
                        sleep(1);
                        if (countOfServedCustomer == store->num_customer_cashier_1) {
                            break;
                        }
                    }
                }

                if (!store->is_start) {
                    answer = -1;
                }
            }
            sleep(1);
            sendto(sockfd, &countOfServedCustomer, sizeof(int), 0, (struct sockaddr *) &client_addr, client_addr_len);
            sleep(1);
            char buffer[BUFFER];
            sprintf(buffer,"Cashier №%d is going home. He served: %d customer\n",
                    cashier_id,
                    countOfServedCustomer);
            sendToObserver(buffer);
            printf(
                    "Cashier №%d is going home. He served: %d customers\n",
                    cashier_id,
                    countOfServedCustomer
            );

            break;
        }
    } else {
        sendto(sockfd, &store->num_customer_cashier_2, sizeof(int), 0, (struct sockaddr *) &client_addr, client_addr_len);
        while (store->is_start) {
            sleep(1);
            char buffer1[BUFFER];
            sprintf(buffer1,"Cashier №%d is ready for work.\n", cashier_id);
            sendToObserver(buffer1);
            printf("Cashier №%d is ready for work.\n", cashier_id);
            for (int i = 0; i < store->num_customer_cashier_2; i++) {
                recvfrom(sockfd, &is_ready, sizeof(int), 0, (struct sockaddr *) &client_addr, &client_addr_len);
                answer = 0;
                if (is_ready == 1) { // Добавленная проверка is_ready
                    if (store->customer[store->customer_cashier_2[i]].state == SECOND && cashier_id == 2) {
                        // отправляем клиенту id покупателя;
                        answer = store->customer[store->customer_cashier_2[i]].id;
                        countOfServedCustomer++;
                        char buffer[BUFFER];
                        sprintf(buffer,"Cashier №%d is served customer №%d\n", cashier_id, answer);
                        sendToObserver(buffer);
                        printf("Cashier №%d is serving customer №%d\n", cashier_id, answer);
                        sendto(sockfd, &answer, sizeof(int), 0, (struct sockaddr *) &client_addr, client_addr_len);
                        sleep(1);
                        if (countOfServedCustomer == store->num_customer_cashier_2) {
                            break;
                        }
                    }
                }

                if (!store->is_start) {
                    answer = -1;
                }
            }
            sleep(1);
            sendto(sockfd, &countOfServedCustomer, sizeof(int), 0, (struct sockaddr *) &client_addr, client_addr_len);
            sleep(1);
            char buffer[BUFFER];
            sprintf(buffer,"Cashier №%d is going home. He served: %d customer\n",
                    cashier_id,
                    countOfServedCustomer);
            sendToObserver(buffer);
            printf(
                    "Cashier №%d is going home. He served: %d customers\n",
                    cashier_id,
                    countOfServedCustomer
            );

            break;
        }
    }
    return NULL;
}

void readCustomers(void *customer_info) {
    CustomerInfo *info = (CustomerInfo *)customer_info;
    int sockfd = info->sockfd;
    struct sockaddr_in client_addr = info->client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int num_customer;
    int ok = 1;
    recvfrom(sockfd, &num_customer, sizeof(int), 0, (struct sockaddr *) &client_addr, &client_addr_len);
    store->num_customer = num_customer;
    store->num_customer_cashier_1 = 0;
    store->num_customer_cashier_2 = 0;
    char buffer[BUFFER];
    sprintf(buffer,"In store waiting %d customers.\n", num_customer);
    sendToObserver(buffer);
    printf("In store waiting %d customers.\n", num_customer);
    srand(time(NULL));
    for (int i = 0; i < num_customer; i++) {
        // cчитываем индексы покупателей
        int id;
        recvfrom(sockfd, &id, sizeof(int), 0, (struct sockaddr *) &client_addr, &client_addr_len);
        store->customer[i].id = id;
        int cashier = rand() % 2 + 1;
        char buffer[BUFFER];
        sprintf(buffer,"Customer №%d is in store and in queue for cashier №%d\n", store->customer[i].id, cashier);
        sendToObserver(buffer);
        printf("Customer №%d is in store and in queue for cashier №%d\n", store->customer[i].id, cashier);
        if (cashier == 1) {
            store->customer_cashier_1[store->num_customer_cashier_1] = i;
            store->num_customer_cashier_1++;
            store->customer[i].state = FIRST;

        } else {
            store->customer_cashier_2[store->num_customer_cashier_2] = i;
            store->num_customer_cashier_2++;
            store->customer[i].state = SECOND;
        }
    }

}

// Function to send console output to the observer client
void sendToObserver(const char *message) {
    int sockfd = observer.sockfd;
    struct sockaddr_in client_addr = observer.client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&client_addr, client_addr_len);
}

// Thread function to handle the observer client connection
void *observerHandler(void *observer_info) {
    ObserverInfo *info = (ObserverInfo *)observer_info;
    int sockfd = info->sockfd;
    struct sockaddr_in client_addr = info->client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[1024];

    while (1) {
        ssize_t bytesRead = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&client_addr, &client_addr_len);
        if (bytesRead <= 0) {
            // Observer client disconnected
            break;
        }

        buffer[bytesRead] = '\0';
        printf("Received message from observer: %s\n", buffer);
    }


    // Terminate the server if the observer client disconnects
    printf("Observer client disconnected. Server is shutting down.\n");
    exit(0);
}


int main(int argc, char const *argv[]) {
    signal(SIGINT, sigfunc);
    signal(SIGTERM, sigfunc);
    unsigned short server_port;
    store = (Store *)malloc(sizeof(Store));
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    if (argc != 2) {
        fprintf(stderr, "Usage:  %s <Server Port>\n", argv[0]);
        exit(1);
    }

    server_port = atoi(argv[1]);

    // Создайте сокет для каждого клиента-кассира
    int sockfd[4];
    for (int i = 0; i < 4; i++) {
        if ((sockfd[i] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }
    }


    // Связывание сокетов с адресом и портом
    struct sockaddr_in server_addr[4];
    for (int i = 0; i < 4; i++) {
        memset(&server_addr[i], 0, sizeof(server_addr[i]));
        server_addr[i].sin_family = AF_INET;
        server_addr[i].sin_addr.s_addr = htonl(INADDR_ANY);
        server_addr[i].sin_port = htons(server_port + i); // Используйте разные порты для каждого клиента-кассира

        if (bind(sockfd[i], (struct sockaddr *) &server_addr[i], sizeof(server_addr[i])) < 0) {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }
    }

    printf("Store server is start\n");
    // Создайте сокет для наблюдателя
    int id_observer;
    recvfrom(sockfd[3], &id_observer, sizeof(int), 0, (struct sockaddr *) &client_addr, &client_addr_len);
    observer.sockfd = sockfd[3];
    observer.client_addr = client_addr;
    if (id_observer == 3) {
        printf("Observer is connected.\n");
        pthread_t observer_thread;
        if (pthread_create(&observer_thread, NULL, observerHandler, (void *)&observer) < 0) {
            perror("could not create observer thread");
            exit(EXIT_FAILURE);
        }
    }

    // Accept connection from customer generator
    int id;
    recvfrom(sockfd[0], &id, sizeof(int), 0, (struct sockaddr *) &client_addr, &client_addr_len);
    customer.sockfd = sockfd[0];
    customer.client_addr = client_addr;
    if (id == 1) {
        printf("Generator is connected.\n");
        readCustomers((void *)&customer);
        printf("All customers are in the queue\n");
    }

    store->is_start = true;
    int count_clients = 0;

    pthread_t tid[2];
    // Handle each new connection in a separate thread
    for (int i = 1; i < 3; i++) {
        // Получение сообщения от клиента-кассира
        int cashier_id;
        recvfrom(sockfd[i], &cashier_id, sizeof(int), 0, (struct sockaddr *) &client_addr, &client_addr_len);

        // Сохранение информации о клиенте-кассире
        cashiers[i - 1].sockfd = sockfd[i];
        cashiers[i - 1].client_addr = client_addr;

        // Создание потока для обработки клиента-кассира
        if (pthread_create(&tid[i - 1], NULL, cashierHandler, (void *)&cashiers[i - 1]) != 0) {
            perror("pthread_create failed");
            exit(EXIT_FAILURE);
        }
        count_clients++;
        printf("Cashier is connected.\n");
    }

    for (int i = 0; i < count_clients; i++) {
        pthread_join(tid[i], NULL);
    }

    free(store);

    printf("All cashier threads have finished. Server is shutting down.\n");

    char message[BUFFER];
    sprintf(message, "Server has finished its work.");
    sendToObserver(message);
    sleep(1);

    return 0;
}
