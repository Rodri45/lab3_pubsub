/*
 * publisher_tcp.c
 * Publicador del sistema pub-sub usando sockets TCP.
 * Se conecta al broker, se registra como publicador de un tema
 * y envía mensajes de eventos deportivos.
 *
 * Compilar: gcc -o publisher_tcp publisher_tcp.c
 * Ejecutar: ./publisher_tcp <IP_BROKER> <TEMA>
 * Ejemplo:  ./publisher_tcp 127.0.0.1 BarcelonaVsRealMadrid
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
        printf("Uso: %s <IP_BROKER> <TEMA>\n", argv[0]);
        printf("Ejemplo: %s 127.0.0.1 BarcelonaVsRealMadrid\n", argv[0]);
        return 1;
    }

    char *broker_ip = argv[1];
    char *topic = argv[2];

    int sock_fd;
    struct sockaddr_in broker_addr;
    char buffer[BUFFER_SIZE];

    // 1. Crear socket TCP
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("[Publisher] Error creando socket");
        exit(EXIT_FAILURE);
    }

    // 2. Configurar dirección del broker
    memset(&broker_addr, 0, sizeof(broker_addr));
    broker_addr.sin_family = AF_INET;
    broker_addr.sin_port = htons(PORT);

    // Convertir IP string a formato binario
    if (inet_pton(AF_INET, broker_ip, &broker_addr.sin_addr) <= 0) {
        perror("[Publisher] Dirección IP inválida");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    // 3. Conectar al broker (TCP handshake de 3 vías ocurre aquí)
    if (connect(sock_fd, (struct sockaddr *)&broker_addr, sizeof(broker_addr)) < 0) {
        perror("[Publisher] Error conectando al broker");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    printf("[Publisher] Conectado al broker %s:%d\n", broker_ip, PORT);

    // 4. Registrarse como publicador: enviar "PUB|tema"
    snprintf(buffer, BUFFER_SIZE, "PUB|%s", topic);
    send(sock_fd, buffer, strlen(buffer), 0);
    printf("[Publisher] Registrado como publicador del tema: %s\n", topic);

    // 5. Enviar mensajes de eventos deportivos
    // Mensajes predefinidos simulando un partido
    const char *eventos[] = {
        "Inicio del partido. Pitazo inicial del arbitro.",
        "Minuto 5: Tiro de esquina para el equipo local.",
        "Minuto 12: Falta en el medio campo.",
        "Minuto 18: GOL! El equipo local se adelanta 1-0.",
        "Minuto 25: Tarjeta amarilla para el jugador #7 visitante.",
        "Minuto 32: Tiro libre peligroso del equipo visitante.",
        "Minuto 38: GOL! El equipo visitante empata 1-1.",
        "Minuto 45: Fin del primer tiempo. Marcador: 1-1.",
        "Minuto 46: Comienza el segundo tiempo.",
        "Minuto 55: Cambio: Entra jugador #11, sale jugador #9 local.",
        "Minuto 63: Tarjeta roja para el jugador #3 visitante!",
        "Minuto 70: Penal a favor del equipo local!",
        "Minuto 71: GOL de penal! El equipo local gana 2-1.",
        "Minuto 80: Contraataque peligroso del visitante.",
        "Minuto 90: Fin del partido. Resultado final: 2-1."
    };

    int num_eventos = sizeof(eventos) / sizeof(eventos[0]);

    printf("\n[Publisher] Enviando %d eventos del partido '%s'...\n\n", num_eventos, topic);

    for (int i = 0; i < num_eventos; i++) {
        // Formato: "tema|mensaje"
        snprintf(buffer, BUFFER_SIZE, "%s|%s\n", topic, eventos[i]);

        if (send(sock_fd, buffer, strlen(buffer), 0) < 0) {
            perror("[Publisher] Error enviando mensaje");
            break;
        }

        printf("[Publisher] Enviado (%d/%d): %s\n", i + 1, num_eventos, eventos[i]);

        // Esperar 2 segundos entre mensajes (simula tiempo real)
        sleep(2);
    }

    printf("\n[Publisher] Todos los eventos enviados. Cerrando conexión.\n");

    // 6. Cerrar socket
    close(sock_fd);
    return 0;
}
