/*
 * Клиент сетевой службы echo (TCP, RFC 862).
 * Использование:
 *   ./echo_client <host> [port]              — интерактивный режим
 *   ./echo_client <host> [port] "сообщение"  — отправить строку и получить эхо
 * По умолчанию порт 7.
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

#define DEFAULT_PORT "7"
#define BUF_SIZE     4096

static int connect_to(const char *host, const char *port)
{
    struct addrinfo hints, *res = NULL, *p = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM; /* TCP */
    hints.ai_protocol = IPPROTO_TCP;

    int rc = getaddrinfo(host, port, &hints, &res);
    if (rc != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
        return -1;
    }

    int sockfd = -1;
    for (p = res; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd < 0) continue;
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == 0)
            break;               /* успешно подключились */
        close(sockfd);
        sockfd = -1;
    }

    freeaddrinfo(res);
    return sockfd;
}

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 4) {
        fprintf(stderr,
                "Использование:\n"
                "  %s <host> [port]              — интерактивный режим\n"
                "  %s <host> [port] \"message\"    — отправить строку\n"
                "Пример: %s 172.16.40.1 \"Hello\"\n",
                argv[0], argv[0], argv[0]);
        return EXIT_FAILURE;
    }

    const char *host    = argv[1];
    const char *port    = DEFAULT_PORT;
    const char *message = NULL;

    if (argc >= 3) {
        /* Если 3-й аргумент — число, считаем его портом, иначе сообщением */
        char *end = NULL;
        long val = strtol(argv[2], &end, 10);
        if (*end == '\0' && val > 0 && val < 65536 && argc == 3) {
            port = argv[2];
        } else if (argc == 3) {
            message = argv[2];
        } else { /* argc == 4 */
            port    = argv[2];
            message = argv[3];
        }
    }

    int sockfd = connect_to(host, port);
    if (sockfd < 0) {
        fprintf(stderr, "Не удалось подключиться к %s:%s\n", host, port);
        return EXIT_FAILURE;
    }

    char buf[BUF_SIZE];
    ssize_t n;

    if (message != NULL) {
        /* Однократная отправка сообщения */
        size_t len = strlen(message);
        if (send(sockfd, message, len, 0) < 0) {
            perror("send");
            close(sockfd);
            return EXIT_FAILURE;
        }
        while ((n = recv(sockfd, buf, sizeof(buf), 0)) > 0)
            fwrite(buf, 1, (size_t)n, stdout);
        if (n < 0) perror("recv");
        printf("\n");
    } else {
        /* Интерактивный режим: строки со stdin -> сервер -> stdout */
        printf("Введите строки (Ctrl+D — выход):\n");
        while (fgets(buf, sizeof(buf), stdin) != NULL) {
            size_t len = strlen(buf);
            if (send(sockfd, buf, len, 0) < 0) { perror("send"); break; }
            n = recv(sockfd, buf, sizeof(buf), 0);
            if (n <= 0) { if (n < 0) perror("recv"); break; }
            fwrite(buf, 1, (size_t)n, stdout);
        }
    }

    close(sockfd);
    return EXIT_SUCCESS;
}
