
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 9090
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: %s <IP_BROKER> <TEMA1,TEMA2,...>\n", argv[0]);
        printf("Ejemplo: %s 127.0.0.1 RealMadridVsManCity\n", argv[0]);
        return 1;
    }

    char *broker_ip = argv[1];
    char *topics = argv[2];

    int sock_fd;
    struct sockaddr_in broker_addr, from_addr;
    socklen_t addr_len = sizeof(broker_addr);
    socklen_t from_len = sizeof(from_addr);
    char buffer[BUFFER_SIZE];

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("[Subscriber UDP] Error creando socket");
        exit(EXIT_FAILURE);
    }

    memset(&broker_addr, 0, sizeof(broker_addr));
    broker_addr.sin_family = AF_INET;
    broker_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, broker_ip, &broker_addr.sin_addr) <= 0) {
        perror("[Subscriber UDP] Dirección IP inválida");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    printf("[Subscriber UDP] Listo, broker en %s:%d\n", broker_ip, PORT);

    snprintf(buffer, BUFFER_SIZE, "SUB|%s", topics);
    sendto(sock_fd, buffer, strlen(buffer), 0,
           (struct sockaddr *)&broker_addr, addr_len);
    printf("[Subscriber UDP] Solicitud de suscripción enviada: %s\n", topics);

    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    memset(buffer, 0, BUFFER_SIZE);
    int bytes = recvfrom(sock_fd, buffer, BUFFER_SIZE - 1, 0,
                         (struct sockaddr *)&from_addr, &from_len);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        printf("[Subscriber UDP] Broker dice: %s", buffer);
    } else {
        printf("[Subscriber UDP] No se recibió confirmación (normal en UDP).\n");
    }

    tv.tv_sec = 0;
    tv.tv_usec = 0;
    setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    printf("[Subscriber UDP] Esperando mensajes...\n\n");

    int msg_count = 0;
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        bytes = recvfrom(sock_fd, buffer, BUFFER_SIZE - 1, 0,
                         (struct sockaddr *)&from_addr, &from_len);

        if (bytes <= 0) {
            if (bytes == 0) {
                printf("[Subscriber UDP] Recibido datagrama vacío.\n");
            }
            continue;
        }

        buffer[bytes] = '\0';
        msg_count++;
        printf("  >> Mensaje #%d: %s", msg_count, buffer);

        if (buffer[bytes - 1] != '\n') {
            printf("\n");
        }
    }

    printf("[Subscriber UDP] Total de mensajes recibidos: %d\n", msg_count);

    close(sock_fd);
    return 0;
}
