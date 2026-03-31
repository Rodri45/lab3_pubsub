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
        printf("Uso: %s <IP_BROKER> <TEMA>\n", argv[0]);
        printf("Ejemplo: %s 127.0.0.1 RealMadridVsManCity\n", argv[0]);
        return 1;
    }

    char *broker_ip = argv[1];
    char *topic = argv[2];

    int sock_fd;
    struct sockaddr_in broker_addr;
    socklen_t addr_len = sizeof(broker_addr);
    char buffer[BUFFER_SIZE];

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("[Publisher UDP] Error creando socket");
        exit(EXIT_FAILURE);
    }

    memset(&broker_addr, 0, sizeof(broker_addr));
    broker_addr.sin_family = AF_INET;
    broker_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, broker_ip, &broker_addr.sin_addr) <= 0) {
        perror("[Publisher UDP] Dirección IP inválida");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }


    printf("[Publisher UDP] Listo para enviar al broker %s:%d\n", broker_ip, PORT);

    snprintf(buffer, BUFFER_SIZE, "PUB|%s", topic);
    sendto(sock_fd, buffer, strlen(buffer), 0,
           (struct sockaddr *)&broker_addr, addr_len);
    printf("[Publisher UDP] Registrado como publicador del tema: %s\n", topic);

    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    memset(buffer, 0, BUFFER_SIZE);
    int bytes = recvfrom(sock_fd, buffer, BUFFER_SIZE - 1, 0, NULL, NULL);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        printf("[Publisher UDP] Broker respondió: %s", buffer);
    } else {
        printf("[Publisher UDP] No se recibió ACK del broker (normal en UDP).\n");
    }

    tv.tv_sec = 0;
    tv.tv_usec = 0;
    setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    const char *eventos[] = {
        "#1 Inicio del partido. Pitazo inicial del arbitro.",
        "#2 Minuto 5: Tiro de esquina para el equipo local.",
        "#3 Minuto 12: Falta en el medio campo.",
        "#4 Minuto 18: GOL! El equipo local se adelanta 1-0.",
        "#5 Minuto 25: Tarjeta amarilla para el jugador #7 visitante.",
        "#6 Minuto 32: Tiro libre peligroso del equipo visitante.",
        "#7 Minuto 38: GOL! El equipo visitante empata 1-1.",
        "#8 Minuto 45: Fin del primer tiempo. Marcador: 1-1.",
        "#9 Minuto 46: Comienza el segundo tiempo.",
        "#10 Minuto 55: Cambio: Entra jugador #11, sale jugador #9 local.",
        "#11 Minuto 63: Tarjeta roja para el jugador #3 visitante!",
        "#12 Minuto 70: Penal a favor del equipo local!",
        "#13 Minuto 71: GOL de penal! El equipo local gana 2-1.",
        "#14 Minuto 80: Contraataque peligroso del visitante.",
        "#15 Minuto 90: Fin del partido. Resultado final: 2-1."
    };

    int num_eventos = sizeof(eventos) / sizeof(eventos[0]);

    printf("\n[Publisher UDP] Enviando %d eventos del partido '%s'...\n\n", num_eventos, topic);

    for (int i = 0; i < num_eventos; i++) {
        snprintf(buffer, BUFFER_SIZE, "MSG|%s|%s", topic, eventos[i]);

        int sent = sendto(sock_fd, buffer, strlen(buffer), 0,
                          (struct sockaddr *)&broker_addr, addr_len);

        if (sent < 0) {
            perror("[Publisher UDP] Error enviando datagrama");
        } else {
            printf("[Publisher UDP] Enviado (%d/%d): %s\n", i + 1, num_eventos, eventos[i]);
        }

        sleep(2);
    }

    printf("\n[Publisher UDP] Todos los eventos enviados. Cerrando.\n");

    close(sock_fd);
    return 0;
}
