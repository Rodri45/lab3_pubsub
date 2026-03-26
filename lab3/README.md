# Laboratorio 3 - Sistema Pub-Sub TCP y UDP
## Infraestructura de Comunicaciones - Universidad de los Andes

### Descripción
Sistema de noticias deportivas en tiempo real implementado con el patrón publicación-suscripción (pub-sub) usando sockets en C, tanto con TCP como con UDP.

### Estructura de archivos
```
lab3/
├── broker_tcp.c        # Broker TCP (servidor central)
├── publisher_tcp.c     # Publicador TCP
├── subscriber_tcp.c    # Suscriptor TCP
├── broker_udp.c        # Broker UDP
├── publisher_udp.c     # Publicador UDP
├── subscriber_udp.c    # Suscriptor UDP
├── Makefile            # Compilación automática
└── README.md           # Este archivo
```

### Compilación
```bash
make          # Compila todo
make tcp      # Solo la versión TCP
make udp      # Solo la versión UDP
make clean    # Elimina ejecutables
```

### Documentación de funciones de sockets utilizadas

Todas las funciones provienen de las librerías estándar de POSIX/BSD sockets incluidas en:
- `<sys/socket.h>` — funciones de socket
- `<arpa/inet.h>` — conversión de direcciones
- `<unistd.h>` — close()
- `<pthread.h>` — hilos POSIX (solo broker TCP)

#### Funciones de socket punto a punto:

| Función | Descripción | Usado en |
|---------|-------------|----------|
| `socket(AF_INET, SOCK_STREAM, 0)` | Crea un socket TCP (orientado a conexión) | broker_tcp, publisher_tcp, subscriber_tcp |
| `socket(AF_INET, SOCK_DGRAM, 0)` | Crea un socket UDP (datagramas) | broker_udp, publisher_udp, subscriber_udp |
| `bind(fd, addr, len)` | Asocia el socket a una dirección IP y puerto local | broker_tcp, broker_udp |
| `listen(fd, backlog)` | Pone el socket TCP en modo escucha pasiva | broker_tcp |
| `accept(fd, addr, len)` | Acepta una nueva conexión TCP entrante | broker_tcp |
| `connect(fd, addr, len)` | Establece conexión TCP con el servidor (3-way handshake) | publisher_tcp, subscriber_tcp |
| `send(fd, buf, len, flags)` | Envía datos por un socket TCP conectado | broker_tcp, publisher_tcp, subscriber_tcp |
| `recv(fd, buf, len, flags)` | Recibe datos de un socket TCP conectado | broker_tcp, publisher_tcp, subscriber_tcp |
| `sendto(fd, buf, len, flags, addr, addrlen)` | Envía datagrama UDP a una dirección específica | broker_udp, publisher_udp, subscriber_udp |
| `recvfrom(fd, buf, len, flags, addr, addrlen)` | Recibe datagrama UDP y obtiene dirección del remitente | broker_udp, publisher_udp, subscriber_udp |
| `close(fd)` | Cierra el socket y libera recursos | Todos |
| `setsockopt(fd, level, opt, val, len)` | Configura opciones del socket (SO_REUSEADDR, SO_RCVTIMEO) | Brokers, publisher_udp, subscriber_udp |
| `inet_pton(AF_INET, str, addr)` | Convierte dirección IP de texto a formato binario | Publishers, Subscribers |
| `inet_ntoa(addr)` | Convierte dirección IP binaria a texto legible | Brokers |
| `htons(port)` | Convierte puerto de host byte order a network byte order | Todos |
| `ntohs(port)` | Convierte puerto de network byte order a host byte order | Brokers |

#### Funciones de hilos POSIX (solo broker TCP):

| Función | Descripción |
|---------|-------------|
| `pthread_create(tid, attr, func, arg)` | Crea un nuevo hilo para manejar un cliente |
| `pthread_detach(tid)` | Marca el hilo como "detached" para limpieza automática |
| `pthread_mutex_lock(mutex)` | Adquiere el mutex para acceso exclusivo a datos compartidos |
| `pthread_mutex_unlock(mutex)` | Libera el mutex |

### Puertos utilizados
- **TCP**: Puerto 8080
- **UDP**: Puerto 9090
