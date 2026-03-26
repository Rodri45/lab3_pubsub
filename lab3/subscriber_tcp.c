/*
 * subscriber_tcp.c
 * Suscriptor del sistema pub-sub usando sockets TCP.
 * Se conecta al broker, se suscribe a uno o varios temas
 * y recibe los mensajes en tiempo real.
 *
 * Compilar: gcc -o subscriber_tcp subscriber_tcp.c
 * Ejecutar: ./subscriber_tcp <IP_BROKER> <TEMA1,TEMA2,...>
 * Ejemplo:  ./subscriber_tcp 127.0.0.1 BarcelonaVsRealMadrid
 *           ./subscriber_tcp 127.0.0.1 BarcelonaVsRealMadrid,AtleticoVsSevilla
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    // Validar argumentos
    if (argc != 3) {
        printf("Uso: %s <IP_BROKER> <TEMA1,TEMA2,...>\n", argv[0]);
        printf("Ejemplo: %s 127.0.0.1 BarcelonaVsRealMadrid\n", argv[0]);
        printf("         %s 127.0.0.1 BarcelonaVsRealMadrid,AtleticoVsSevilla\n", argv[0]);
        return 1;
    }

    char *broker_ip = argv[1];
    char *topics = argv[2];

    int sock_fd;
    struct sockaddr_in broker_addr;
    char buffer[BUFFER_SIZE];

    // 1. Crear socket TCP
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("[Subscriber] Error creando socket");
        exit(EXIT_FAILURE);
    }

    // 2. Configurar dirección del broker
    memset(&broker_addr, 0, sizeof(broker_addr));
    broker_addr.sin_family = AF_INET;
    broker_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, broker_ip, &broker_addr.sin_addr) <= 0) {
        perror("[Subscriber] Dirección IP inválida");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    // 3. Conectar al broker
    if (connect(sock_fd, (struct sockaddr *)&broker_addr, sizeof(broker_addr)) < 0) {
        perror("[Subscriber] Error conectando al broker");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    printf("[Subscriber] Conectado al broker %s:%d\n", broker_ip, PORT);

    // 4. Registrarse como suscriptor: enviar "SUB|tema1,tema2,..."
    snprintf(buffer, BUFFER_SIZE, "SUB|%s", topics);
    send(sock_fd, buffer, strlen(buffer), 0);
    printf("[Subscriber] Solicitud de suscripción enviada: %s\n", topics);

    // 5. Recibir confirmación
    memset(buffer, 0, BUFFER_SIZE);
    int bytes = recv(sock_fd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        printf("[Subscriber] Broker dice: %s", buffer);
    }

    printf("[Subscriber] Esperando mensajes...\n\n");

    // 6. Bucle de recepción de mensajes
    int msg_count = 0;
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        bytes = recv(sock_fd, buffer, BUFFER_SIZE - 1, 0);

        if (bytes <= 0) {
            if (bytes == 0) {
                printf("\n[Subscriber] El broker cerró la conexión.\n");
            } else {
                perror("[Subscriber] Error recibiendo mensaje");
            }
            break;
        }

        buffer[bytes] = '\0';
        msg_count++;
        printf("  >> Mensaje #%d: %s", msg_count, buffer);

        // Verificar si el buffer no termina en newline
        if (buffer[bytes - 1] != '\n') {
            printf("\n");
        }
    }

    printf("[Subscriber] Total de mensajes recibidos: %d\n", msg_count);

    // 7. Cerrar socket
    close(sock_fd);
    return 0;
}
