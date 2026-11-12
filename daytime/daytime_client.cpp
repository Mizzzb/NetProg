/*
 * Клиент сетевой службы daytime (UDP, RFC 867).
 * Использование: ./daytime_client <host> [port]
 * По умолчанию порт 13.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define DEFAULT_PORT "13"
#define BUF_SIZE     1024

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Использование: %s <host> [port]\n", argv[0]);
        fprintf(stderr, "Пример: %s 172.16.40.1\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *host = argv[1];
    const char *port = (argc == 3) ? argv[2] : DEFAULT_PORT;

    struct addrinfo hints, *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;   /* IPv4 или IPv6 */
    hints.ai_socktype = SOCK_DGRAM;  /* UDP */
    hints.ai_protocol = IPPROTO_UDP;

    int rc = getaddrinfo(host, port, &hints, &res);
    if (rc != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
        return EXIT_FAILURE;
    }

    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        perror("socket");
        freeaddrinfo(res);
        return EXIT_FAILURE;
    }

    /* Устанавливаем таймаут на приём, чтобы не ждать бесконечно */
    struct timeval tv = { .tv_sec = 5, .tv_usec = 0 };
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    /* Отправляем пустой датаграм (1 байт) — типичный запрос к daytime */
    const char request = '\0';
    ssize_t sent = sendto(sockfd, &request, 1, 0,
                          res->ai_addr, res->ai_addrlen);
    if (sent < 0) {
        perror("sendto");
        close(sockfd);
        freeaddrinfo(res);
        return EXIT_FAILURE;
    }

    char buf[BUF_SIZE];
    struct sockaddr_storage from;
    socklen_t from_len = sizeof(from);

    ssize_t n = recvfrom(sockfd, buf, sizeof(buf) - 1, 0,
                         (struct sockaddr *)&from, &from_len);
    if (n < 0) {
        perror("recvfrom");
        close(sockfd);
        freeaddrinfo(res);
        return EXIT_FAILURE;
    }

    buf[n] = '\0';

    /* Печатаем адрес ответившего сервера */
    char hoststr[NI_MAXHOST], servstr[NI_MAXSERV];
    if (getnameinfo((struct sockaddr *)&from, from_len,
                    hoststr, sizeof(hoststr),
                    servstr, sizeof(servstr),
                    NI_NUMERICHOST | NI_NUMERICSERV) == 0) {
        printf("Ответ от %s:%s:\n", hoststr, servstr);
    }

    printf("%s", buf);
    if (n == 0 || buf[n - 1] != '\n')
        printf("\n");

    close(sockfd);
    freeaddrinfo(res);
    return EXIT_SUCCESS;
}
