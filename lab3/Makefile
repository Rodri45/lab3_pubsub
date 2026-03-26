# Makefile - Laboratorio 3: Sistema Pub-Sub TCP y UDP
# Compilar todo: make
# Compilar solo TCP: make tcp
# Compilar solo UDP: make udp
# Limpiar: make clean

CC = gcc
CFLAGS = -Wall -Wextra -g

# Todos los ejecutables
all: tcp udp

# --- TCP ---
tcp: broker_tcp publisher_tcp subscriber_tcp

broker_tcp: broker_tcp.c
	$(CC) $(CFLAGS) -o broker_tcp broker_tcp.c -lpthread

publisher_tcp: publisher_tcp.c
	$(CC) $(CFLAGS) -o publisher_tcp publisher_tcp.c

subscriber_tcp: subscriber_tcp.c
	$(CC) $(CFLAGS) -o subscriber_tcp subscriber_tcp.c

# --- UDP ---
udp: broker_udp publisher_udp subscriber_udp

broker_udp: broker_udp.c
	$(CC) $(CFLAGS) -o broker_udp broker_udp.c

publisher_udp: publisher_udp.c
	$(CC) $(CFLAGS) -o publisher_udp publisher_udp.c

subscriber_udp: subscriber_udp.c
	$(CC) $(CFLAGS) -o subscriber_udp subscriber_udp.c

# --- Limpieza ---
clean:
	rm -f broker_tcp publisher_tcp subscriber_tcp
	rm -f broker_udp publisher_udp subscriber_udp

.PHONY: all tcp udp clean
