#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    unsigned short server_port;
    const char *server_ip;

    int sock;
    struct sockaddr_in serv_addr;


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

    // Отправляем пустой пакет для установления соединения с сервером
    int identificator = 3;
    sendto(sock, &identificator, sizeof(int), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    // Receive and print messages from the server console
    char buffer[1024];
    int bytes_received;
    socklen_t addr_len = sizeof(serv_addr);

    while ((bytes_received = recvfrom(sock, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr *) &serv_addr, &addr_len)) > 0) {
        buffer[bytes_received] = '\0';  // Добавляем завершающий нулевой символ
        // Обрабатываем полученное сообщение
        if (strcmp(buffer, "Server has finished its work.") == 0) {
            printf("Server has finished its work.\n");
            break;
        }
        printf("%s", buffer);
    }

    // Close the observer client socket
    close(sock);

    return 0;
}
