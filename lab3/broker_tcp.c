#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define PORT 8080
#define MAX_CLIENTS 50
#define BUFFER_SIZE 1024
#define MAX_TOPICS 10
#define TOPIC_LEN 64


typedef enum { CLIENT_PUB, CLIENT_SUB } client_type_t;

typedef struct {
    int fd;                          
    client_type_t type;              
    char topics[MAX_TOPICS][TOPIC_LEN]; 
    int topic_count;                
    int active;                      
} client_t;



client_t clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

int add_client(int fd) {
    pthread_mutex_lock(&clients_mutex);
    if (client_count >= MAX_CLIENTS) {
        pthread_mutex_unlock(&clients_mutex);
        return -1;
    }
    int idx = client_count;
    clients[idx].fd = fd;
    clients[idx].type = CLIENT_PUB; e
    clients[idx].topic_count = 0;
    clients[idx].active = 1;
    client_count++;
    pthread_mutex_unlock(&clients_mutex);
    return idx;
}

void remove_client(int idx) {
    pthread_mutex_lock(&clients_mutex);
    clients[idx].active = 0;
    close(clients[idx].fd);
    pthread_mutex_unlock(&clients_mutex);
}

void broadcast_to_subscribers(const char *topic, const char *message) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].active && clients[i].type == CLIENT_SUB) {
            for (int t = 0; t < clients[i].topic_count; t++) {
                if (strcmp(clients[i].topics[t], topic) == 0) {
                    if (send(clients[i].fd, message, strlen(message), 0) < 0) {
                        perror("[Broker] Error enviando a suscriptor");
                    } else {
                        printf("[Broker] Reenviado a suscriptor (fd=%d): %s",
                               clients[i].fd, message);
                    }
                    break; 
                }
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}


void *handle_client(void *arg) {
    int idx = *(int *)arg;
    free(arg);

    int fd = clients[idx].fd;
    char buffer[BUFFER_SIZE];

    memset(buffer, 0, BUFFER_SIZE);
    int bytes = recv(fd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes <= 0) {
        printf("[Broker] Cliente desconectado antes de identificarse.\n");
        remove_client(idx);
        return NULL;
    }
    buffer[bytes] = '\0';

    char *newline = strchr(buffer, '\n');
    if (newline) *newline = '\0';

    if (strncmp(buffer, "PUB|", 4) == 0) {
        clients[idx].type = CLIENT_PUB;
        strncpy(clients[idx].topics[0], buffer + 4, TOPIC_LEN - 1);
        clients[idx].topic_count = 1;
        printf("[Broker] Publicador conectado (fd=%d), tema: %s\n", fd, clients[idx].topics[0]);

    } else if (strncmp(buffer, "SUB|", 4) == 0) {
        clients[idx].type = CLIENT_SUB;
        char *token = strtok(buffer + 4, ",");
        int t = 0;
        while (token != NULL && t < MAX_TOPICS) {
            strncpy(clients[idx].topics[t], token, TOPIC_LEN - 1);
            clients[idx].topics[t][TOPIC_LEN - 1] = '\0';
            t++;
            token = strtok(NULL, ",");
        }
        clients[idx].topic_count = t;
        printf("[Broker] Suscriptor conectado (fd=%d), temas:", fd);
        for (int i = 0; i < t; i++) printf(" %s", clients[idx].topics[i]);
        printf("\n");

        const char *ack = "Suscripcion exitosa.\n";
        send(fd, ack, strlen(ack), 0);

    } else {
        printf("[Broker] Mensaje de registro inválido: %s\n", buffer);
        remove_client(idx);
        return NULL;
    }

    if (clients[idx].type == CLIENT_PUB) {
        while (1) {
            memset(buffer, 0, BUFFER_SIZE);
            bytes = recv(fd, buffer, BUFFER_SIZE - 1, 0);
            if (bytes <= 0) {
                printf("[Broker] Publicador (fd=%d) desconectado.\n", fd);
                break;
            }
            buffer[bytes] = '\0';
            printf("[Broker] Recibido de publicador (fd=%d): %s", fd, buffer);

            char *sep = strchr(buffer, '|');
            if (sep) {
                *sep = '\0';
                char *topic = buffer;
                char *msg = sep + 1;

                char full_msg[BUFFER_SIZE];
                snprintf(full_msg, BUFFER_SIZE, "[%s] %s", topic, msg);
                broadcast_to_subscribers(topic, full_msg);
            }
        }
    } else {
       
        while (1) {
            memset(buffer, 0, BUFFER_SIZE);
            bytes = recv(fd, buffer, BUFFER_SIZE - 1, 0);
            if (bytes <= 0) {
                printf("[Broker] Suscriptor (fd=%d) desconectado.\n", fd);
                break;
            }
        }
    }

    remove_client(idx);
    return NULL;
}


int main() {
    int server_fd, new_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("[Broker] Error creando socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;          
    server_addr.sin_addr.s_addr = INADDR_ANY;  
    server_addr.sin_port = htons(PORT);        

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("[Broker] Error en bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("[Broker] Error en listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("=== BROKER TCP iniciado en puerto %d ===\n", PORT);
    printf("Esperando conexiones de publicadores y suscriptores...\n\n");

    while (1) {
        new_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (new_fd < 0) {
            perror("[Broker] Error en accept");
            continue;
        }

        printf("[Broker] Nueva conexión desde %s:%d (fd=%d)\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), new_fd);

        int idx = add_client(new_fd);
        if (idx < 0) {
            printf("[Broker] Máximo de clientes alcanzado.\n");
            close(new_fd);
            continue;
        }

        int *arg = malloc(sizeof(int));
        *arg = idx;
        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, arg);
        pthread_detach(tid); 
    }

    close(server_fd);
    return 0;
}
