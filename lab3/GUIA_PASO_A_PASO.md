# GUÍA PASO A PASO — Laboratorio 3
## Sistema Pub-Sub con Sockets TCP y UDP en VMware Workstation Pro

---

## PARTE 1: Preparar la máquina virtual

### 1.1 Crear/abrir tu VM Linux en VMware
Si ya tienes una VM con Ubuntu, ábrela. Si no, crea una con Ubuntu 22.04 o 24.04.

### 1.2 Instalar dependencias
Abre una terminal en la VM y ejecuta:

```bash
sudo apt update
sudo apt install -y build-essential gcc wireshark net-tools
```

Cuando pregunte si usuarios no-root pueden capturar paquetes, selecciona **Sí**.

Luego agrega tu usuario al grupo wireshark:

```bash
sudo usermod -aG wireshark $USER
```

**Cierra sesión y vuelve a entrar** para que el cambio surta efecto.

---

## PARTE 2: Transferir y organizar los archivos

### 2.1 Crear el directorio del lab

```bash
mkdir -p ~/lab3
cd ~/lab3
```

### 2.2 Copiar los archivos a la VM
Puedes usar VMware Shared Folders, copiar por USB, o simplemente crear los archivos manualmente. Si usas shared folders:

```bash
cp /mnt/hgfs/shared/lab3/* ~/lab3/
```

O si prefieres clonar desde GitHub, sube los archivos a un repo y haz `git clone`.

### 2.3 Verificar que están todos los archivos

```bash
ls -la ~/lab3/
```

Debes ver:
```
broker_tcp.c    publisher_tcp.c    subscriber_tcp.c
broker_udp.c    publisher_udp.c    subscriber_udp.c
Makefile        README.md
```

---

## PARTE 3: Compilar

```bash
cd ~/lab3
make
```

Esto genera 6 ejecutables. Verifica:

```bash
ls -la broker_tcp publisher_tcp subscriber_tcp broker_udp publisher_udp subscriber_udp
```

Si ves los 6 archivos, todo bien. Si hay errores de compilación, compila individualmente para ver cuál falla:

```bash
gcc -Wall -o broker_tcp broker_tcp.c -lpthread
gcc -Wall -o publisher_tcp publisher_tcp.c
gcc -Wall -o subscriber_tcp subscriber_tcp.c
gcc -Wall -o broker_udp broker_udp.c
gcc -Wall -o publisher_udp publisher_udp.c
gcc -Wall -o subscriber_udp subscriber_udp.c
```

---

## PARTE 4: Pruebas TCP

Necesitas **al menos 5 terminales abiertas** (usa Ctrl+Alt+T para abrir nuevas, o usa `tmux`).

### Terminal 1: Iniciar el Broker TCP

```bash
cd ~/lab3
./broker_tcp
```

Verás:
```
=== BROKER TCP iniciado en puerto 8080 ===
Esperando conexiones de publicadores y suscriptores...
```

### Terminal 2: Suscriptor 1 (sigue el partido BarcelonaVsRealMadrid)

```bash
cd ~/lab3
./subscriber_tcp 127.0.0.1 BarcelonaVsRealMadrid
```

### Terminal 3: Suscriptor 2 (sigue DOS partidos)

```bash
cd ~/lab3
./subscriber_tcp 127.0.0.1 BarcelonaVsRealMadrid,AtleticoVsSevilla
```

### Terminal 4: Publicador 1 (transmite BarcelonaVsRealMadrid)

```bash
cd ~/lab3
./publisher_tcp 127.0.0.1 BarcelonaVsRealMadrid
```

### Terminal 5: Publicador 2 (transmite AtleticoVsSevilla)

```bash
cd ~/lab3
./publisher_tcp 127.0.0.1 AtleticoVsSevilla
```

### ¿Qué debes observar?

- **Suscriptor 1** recibe solo mensajes de BarcelonaVsRealMadrid.
- **Suscriptor 2** recibe mensajes de AMBOS partidos.
- **El Broker** muestra en su terminal todos los mensajes recibidos y reenviados.
- Los 15 mensajes de cada publicador llegan completos y en orden.

**Toma capturas de pantalla de las 5 terminales** para el informe.

### Para parar todo
- Los publicadores terminan solos después de enviar los 15 mensajes.
- Los suscriptores se quedan esperando; ciérralos con **Ctrl+C**.
- El broker se cierra con **Ctrl+C**.

---

## PARTE 5: Pruebas UDP

Mismo esquema pero con los ejecutables UDP (puerto 9090).

### Terminal 1: Broker UDP

```bash
cd ~/lab3
./broker_udp
```

### Terminal 2: Suscriptor UDP 1

```bash
cd ~/lab3
./subscriber_udp 127.0.0.1 RealMadridVsManCity
```

### Terminal 3: Suscriptor UDP 2

```bash
cd ~/lab3
./subscriber_udp 127.0.0.1 RealMadridVsManCity,AtleticoVsSevilla
```

### Terminal 4: Publicador UDP 1

```bash
cd ~/lab3
./publisher_udp 127.0.0.1 RealMadridVsManCity
```

### Terminal 5: Publicador UDP 2

```bash
cd ~/lab3
./publisher_udp 127.0.0.1 AtleticoVsSevilla
```

**Toma capturas de pantalla** de las 5 terminales.

---

## PARTE 6: Captura con Wireshark

### 6.1 Captura TCP

**IMPORTANTE:** Inicia Wireshark ANTES de ejecutar los programas.

```bash
wireshark &
```

1. Selecciona la interfaz **Loopback: lo** (porque estamos en localhost).
2. En el filtro de captura escribe: `port 8080`
3. Haz clic en **Start capturing** (el botón de aleta de tiburón azul).
4. Ahora ejecuta el broker, suscriptores y publicadores TCP (como en la Parte 4).
5. Espera a que los publicadores terminen.
6. En Wireshark: **File → Save As → tcp_pubsub.pcap**

### 6.2 Captura UDP

1. Detén la captura anterior (**Capture → Stop**).
2. Inicia una nueva captura en **Loopback: lo**.
3. Filtro de captura: `port 9090`
4. Ejecuta el broker, suscriptores y publicadores UDP (como en la Parte 5).
5. Espera a que terminen.
6. Guardar: **File → Save As → udp_pubsub.pcap**

### 6.3 Qué observar en Wireshark

**Para TCP (filtro de display: `tcp.port == 8080`):**
- Verás los paquetes **SYN, SYN-ACK, ACK** del handshake de 3 vías al inicio.
- Los datos van en paquetes **PSH, ACK**.
- Al final verás **FIN, ACK** para cerrar la conexión.
- Los paquetes ACK confirman la recepción (confiabilidad).
- Las cabeceras TCP son de **20 bytes mínimo**.

**Para UDP (filtro de display: `udp.port == 9090`):**
- No hay handshake. Los datos se envían directamente.
- Cada mensaje es un datagrama independiente.
- No hay ACKs ni confirmaciones.
- Las cabeceras UDP son de solo **8 bytes**.
- Puede haber pérdida si se satura la red (en localhost es difícil de ver, pero conceptualmente es así).

**Toma capturas de pantalla de Wireshark** mostrando:
- El handshake TCP (SYN, SYN-ACK, ACK).
- Los paquetes de datos TCP con ACKs.
- Los datagramas UDP (sin handshake).
- Compara el tamaño de cabeceras.

---

## PARTE 7: Tabla comparativa para el informe

| Criterio | TCP | UDP |
|----------|-----|-----|
| **Confiabilidad** | Sí. Usa ACKs y retransmisiones. Todos los mensajes llegan. | No. Los datagramas pueden perderse sin aviso. |
| **Orden de entrega** | Sí. Los números de secuencia garantizan el orden. | No. Los datagramas pueden llegar desordenados. |
| **Pérdida de mensajes** | No hay pérdida (el protocolo retransmite). | Puede haber pérdida; no hay mecanismo de recuperación. |
| **Overhead de cabeceras** | 20+ bytes por segmento + handshake + ACKs. | Solo 8 bytes por datagrama. Mucho menor. |
| **Establecimiento de conexión** | Requiere 3-way handshake (SYN, SYN-ACK, ACK). | No requiere conexión previa. |
| **Velocidad** | Más lento por el overhead de confiabilidad. | Más rápido al no tener overhead de control. |
| **Uso en el broker** | Un hilo por conexión; mayor uso de CPU/memoria. | Un solo socket para todo; menor uso de recursos. |

---

## PARTE 8: Respuestas a las preguntas de análisis

Aquí van las ideas clave para responder cada pregunta. **Redáctalas con tus propias palabras** y agrega evidencia de tus capturas.

### Pregunta 1: ¿Qué si hubiera 100 partidos simultáneos?
- **TCP:** El broker necesitaría manejar cientos de conexiones simultáneas (hilos o select/epoll). Alto consumo de memoria y CPU por los estados de conexión. El 3-way handshake de cada conexión agrega latencia.
- **UDP:** El broker usa un solo socket. El overhead es menor, pero pierde confiabilidad. A mucho volumen, el buffer del socket se podría llenar y perder datagramas.

### Pregunta 2: Si un suscriptor no recibe un gol en UDP
- En UDP no hay retransmisión: el gol se pierde para siempre para ese suscriptor. La aplicación no se entera. Esto sería crítico en una app real. TCP retransmite automáticamente hasta que el receptor confirme.

### Pregunta 3: ¿TCP o UDP para seguimiento en vivo?
- **TCP** es más adecuado para eventos críticos (goles, tarjetas) donde perder un mensaje es inaceptable. UDP podría usarse para datos de baja prioridad (estadísticas en tiempo real) donde la velocidad importa más que la completitud.

### Pregunta 4: Overhead en Wireshark
- Muestra capturas de pantalla comparando las cabeceras. TCP tiene 20+ bytes por segmento más los paquetes de ACK adicionales. UDP tiene solo 8 bytes y no genera tráfico extra de control.

### Pregunta 5: Marcador desordenado en UDP (2-1 antes que 1-1)
- El usuario vería información incorrecta temporalmente, lo cual es muy confuso. Solución a nivel de aplicación: agregar un número de secuencia o timestamp a cada mensaje y que el suscriptor los reordene antes de mostrarlos.

### Pregunta 6: Más suscriptores en un mismo partido
- **TCP:** Cada suscriptor requiere una conexión dedicada. El broker debe iterar sobre más conexiones para hacer broadcast. Más hilos, más memoria.
- **UDP:** Solo cambia la cantidad de `sendto()` por mensaje. Es más ligero, pero hay más riesgo de pérdida por saturación del buffer.

### Pregunta 7: Broker se detiene inesperadamente
- **TCP:** Los suscriptores detectan la desconexión inmediatamente (recv retorna 0 o error). Pueden intentar reconectarse.
- **UDP:** Los suscriptores no se enteran. Se quedan bloqueados en `recvfrom()` sin saber que el broker cayó. Necesitarían un mecanismo de heartbeat a nivel de aplicación.

### Pregunta 8: Sincronización de actualizaciones críticas
- Ninguno garantiza sincronización perfecta, pero **TCP** es más predecible porque cada suscriptor tiene su propia conexión confiable. En UDP, el broker puede enviar a todos "al mismo tiempo" con sendto() pero sin garantía de entrega.

### Pregunta 9: CPU y memoria del broker
- **TCP:** Usa más CPU (cambio de contexto entre hilos) y más memoria (un hilo + buffers por conexión). Puedes verificarlo con `top` o `htop` mientras corren las pruebas.
- **UDP:** Un solo hilo, un solo socket. Menor uso de recursos pero sin control de flujo.

### Pregunta 10: Sistema real para millones de usuarios
- **Combinación:** TCP para eventos críticos (goles, tarjetas) donde la confiabilidad es esencial. UDP para datos de alta frecuencia y baja criticidad (posesión, estadísticas). Además, en producción se usaría multicast UDP para eficiencia, CDNs, y brokers distribuidos (como Kafka o RabbitMQ).

---

## Comandos útiles adicionales

```bash
# Ver procesos usando los puertos
sudo lsof -i :8080
sudo lsof -i :9090

# Matar procesos en un puerto específico
sudo fuser -k 8080/tcp
sudo fuser -k 9090/udp

# Monitorear uso de CPU/memoria
htop

# Capturar tráfico desde terminal (alternativa a Wireshark GUI)
sudo tcpdump -i lo port 8080 -w tcp_pubsub.pcap
sudo tcpdump -i lo port 9090 -w udp_pubsub.pcap
```

---

## Checklist de entregables

- [ ] Informe PDF con capturas de pantalla y respuestas
- [ ] Link a archivos .pcap (tcp_pubsub.pcap, udp_pubsub.pcap) en Google Drive o GitHub
- [ ] Carpeta comprimida con los 6 archivos .c, Makefile, y README.md
- [ ] Tabla comparativa TCP vs UDP
- [ ] Respuestas a las 10 preguntas de análisis con evidencia
