#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 9090
#define BUFFER_SIZE 1024
#define MAX_SUBSCRIBERS 50
#define MAX_TOPICS 10
#define TOPIC_LEN 64


// Estructura para representar un suscriptor
typedef struct {
    struct sockaddr_in addr;               
    char topics[MAX_TOPICS][TOPIC_LEN];    
    int topic_count;                       
    int active;                            
} subscriber_t;

// Lista de suscriptores
subscriber_t subscribers[MAX_SUBSCRIBERS];
int sub_count = 0;
// Busca un suscriptor por su dirección
int find_subscriber(struct sockaddr_in *addr) {
    for (int i = 0; i < sub_count; i++) {
        if (subscribers[i].active &&
            subscribers[i].addr.sin_addr.s_addr == addr->sin_addr.s_addr &&
            subscribers[i].addr.sin_port == addr->sin_port) {
            return i;
        }
    }
    return -1;
}
// Agrega un nuevo suscriptor a la lista
int add_subscriber(struct sockaddr_in *addr) {
    if (sub_count >= MAX_SUBSCRIBERS) return -1;
    int idx = sub_count;
    subscribers[idx].addr = *addr;
    subscribers[idx].topic_count = 0;
    subscribers[idx].active = 1;
    sub_count++;
    return idx;
}
// Reenvía un mensaje a todos los suscriptores interesados en el tema
void broadcast_to_subscribers(int sock_fd, const char *topic, const char *message) {
    for (int i = 0; i < sub_count; i++) {
        if (!subscribers[i].active) continue;
        for (int t = 0; t < subscribers[i].topic_count; t++) {
            if (strcmp(subscribers[i].topics[t], topic) == 0) {
                int sent = sendto(sock_fd, message, strlen(message), 0,
                                  (struct sockaddr *)&subscribers[i].addr,
                                  sizeof(subscribers[i].addr));
                if (sent < 0) {
                    perror("[Broker UDP] Error enviando a suscriptor");
                } else {
                    printf("[Broker UDP] Reenviado a %s:%d -> %s",
                           inet_ntoa(subscribers[i].addr.sin_addr),
                           ntohs(subscribers[i].addr.sin_port), message);
                }
                break;
            }
        }
    }
}

// Función principal del broker UDP
int main() {
    int sock_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("[Broker UDP] Error creando socket");
        exit(EXIT_FAILURE);
    }
// Permitir reutilizar la dirección
    int opt = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("[Broker UDP] Error en bind");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
// Mensaje de inicio
    printf("=== BROKER UDP iniciado en puerto %d ===\n", PORT);
    printf("Esperando datagramas de publicadores y suscriptores...\n\n");

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recvfrom(sock_fd, buffer, BUFFER_SIZE - 1, 0,
                             (struct sockaddr *)&client_addr, &addr_len);

        if (bytes <= 0) continue;
        buffer[bytes] = '\0';

        char *nl = strchr(buffer, '\n');
        if (nl) *nl = '\0';

        printf("[Broker UDP] Recibido de %s:%d -> %s\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);

        if (strncmp(buffer, "PUB|", 4) == 0) {
            printf("[Broker UDP] Publicador registrado, tema: %s\n", buffer + 4);
            const char *ack = "ACK_PUB\n";
            sendto(sock_fd, ack, strlen(ack), 0,
                   (struct sockaddr *)&client_addr, addr_len);

        } else if (strncmp(buffer, "SUB|", 4) == 0) {
            int idx = find_subscriber(&client_addr);
            if (idx < 0) {
                idx = add_subscriber(&client_addr);
            }
            if (idx >= 0) {
                char topics_buf[BUFFER_SIZE];
                strncpy(topics_buf, buffer + 4, BUFFER_SIZE - 1);
                char *token = strtok(topics_buf, ",");
                subscribers[idx].topic_count = 0;
                while (token != NULL && subscribers[idx].topic_count < MAX_TOPICS) {
                    strncpy(subscribers[idx].topics[subscribers[idx].topic_count],
                            token, TOPIC_LEN - 1);
                    subscribers[idx].topic_count++;
                    token = strtok(NULL, ",");
                }
                printf("[Broker UDP] Suscriptor registrado (%s:%d), temas:",
                       inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                for (int i = 0; i < subscribers[idx].topic_count; i++)
                    printf(" %s", subscribers[idx].topics[i]);
                printf("\n");

                const char *ack = "Suscripcion exitosa.\n";
                sendto(sock_fd, ack, strlen(ack), 0,
                       (struct sockaddr *)&client_addr, addr_len);
            }

        } else if (strncmp(buffer, "MSG|", 4) == 0) {
            char *topic_start = buffer + 4;
            char *sep = strchr(topic_start, '|');
            if (sep) {
                *sep = '\0';
                char *topic = topic_start;
                char *content = sep + 1;

                char full_msg[BUFFER_SIZE];
                snprintf(full_msg, BUFFER_SIZE, "[%s] %s\n", topic, content);
                broadcast_to_subscribers(sock_fd, topic, full_msg);
            }
        }
    }

    close(sock_fd);
    return 0;
}
