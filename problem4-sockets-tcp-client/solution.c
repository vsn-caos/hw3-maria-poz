#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

ssize_t send_all(int sockfd, const uint8_t *buf, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t sent = send(sockfd, buf + total, len - total, 0);
        if (sent <= 0) {
            return sent; 
        }
        total += sent;
    }
    return total;
}

ssize_t recv_all(int sockfd, uint8_t *buf, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t received = recv(sockfd, buf + total, len - total, 0);
        if (received <= 0) {
            return received; 
        }
        total += received;
    }
    return total;
}

// Программе передаются два аргумента:
//   argv[1] — IPv4-адрес сервера в десятичной записи (например, "127.0.0.1")
//   argv[2] — номер порта
//
// Программа должна:
//   1. Установить TCP-соединение с указанным сервером.
//   2. В цикле читать со stdin целые знаковые числа в текстовом формате.
//   3. Отправлять каждое число на сервер в бинарном виде (int32, Little Endian).
//   4. Получать от сервера int32 LE в ответ и выводить его в stdout в текстовом виде.
//   5. Если сервер закрыл соединение — завершиться с кодом возврата 0.

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <ipv4_addr> <port>\n", argv[0]);
        return 1;
    }

    const char *ip_str = argv[1];
    int port = atoi(argv[2]);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip_str, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }

    int32_t val;
    while (scanf("%d", &val) == 1) {
        uint8_t out_buf[4];
        
        out_buf[0] = (val >> 0) & 0xFF;
        out_buf[1] = (val >> 8) & 0xFF;
        out_buf[2] = (val >> 16) & 0xFF;
        out_buf[3] = (val >> 24) & 0xFF;

        if (send_all(sock, out_buf, 4) <= 0) {
            break; 
        }

        uint8_t in_buf[4];
        if (recv_all(sock, in_buf, 4) <= 0) {
            break; 
        }

        int32_t res = (int32_t)(
            ((uint32_t)in_buf[0] << 0)  |
            ((uint32_t)in_buf[1] << 8)  |
            ((uint32_t)in_buf[2] << 16) |
            ((uint32_t)in_buf[3] << 24)
        );

        printf("%d\n", res);
        fflush(stdout);
    }

    close(sock);
    
    return 0;
}
