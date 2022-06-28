/* 
Projecto de SD - Fase 4
Grupo: 45
António Pereira - 50320
Guilherme Lopes - 52761
Ricardo Banon - 42035
*/

#include "network_server.h"
#include "inet.h"
#include <poll.h>
#include <fcntl.h>
#include <stdio.h>

#define NFDESC 4
#define TIMEOUT 50

int portS;
struct sockaddr_in server;
struct pollfd connections[NFDESC];

/* Função para preparar uma socket de receção de pedidos de ligação
 * num determinado porto.
 * Retornar descritor do socket (OK) ou -1 (erro).
 */
int network_server_init(short port) {
    int socket_fd;
    portS = (int) port;
    if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        perror("Erro ao criar socket");
        return -1;
    }
    int sim;
    sim = 1;
    if(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (int *)&sim, sizeof(sim)) < 0) {
        perror("Erro SO_REUSEADOR");
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(portS);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(socket_fd, (struct sockaddr *) &server, sizeof(server)) < 0){
        perror("Erro ao fazer bind");
        close(socket_fd);
        return -1;
    }
    if(listen(socket_fd, 0) < 0){
        perror("Erro ao executar listen");
        close(socket_fd);
        return -1;
    }

    return socket_fd;
}

/* Esta função deve:
 * - Ler os bytes da rede, a partir do client_socket indicado;
 * - De-serializar estes bytes e construir a mensagem com o pedido,
 *   reservando a memória necessária para a estrutura message_t.
 */
struct message_t *network_receive(int client_socket) {
    uint8_t *buf; 
    int packSize;
    char sizerc[4];
    packSize = read_all(client_socket, sizerc, sizeof(sizerc));
    if(strcmp(sizerc,"quit") == 0) {
        return NULL;
    }
    packSize = atoi(sizerc);
    buf = malloc(packSize);
    if(buf == NULL) {
        return NULL;
    }
    packSize = read_all(client_socket, buf, packSize);
    struct _MessageT *msgRcv = NULL;
    msgRcv = message_t__unpack(NULL, packSize, buf);

    if(msgRcv == NULL) {
        return NULL;
    }

    struct message_t *res;
    res = (struct message_t *) malloc (sizeof(struct message_t));
    res->message = (struct _MessageT *) malloc (sizeof(struct _MessageT));
    res->message->opcode = msgRcv->opcode;
    res->message->c_type = msgRcv->c_type;
    res->message->data_size = msgRcv->data_size;
    res->message->key = strdup(msgRcv->key);
    res->message->sizeheight = msgRcv->sizeheight;
    res->message->keys = msgRcv->keys;
    res->message->data = strdup(msgRcv->data);
    res->message->op = msgRcv->op;
    free(buf);
    message_t__free_unpacked(msgRcv, NULL);

    return res;
}

/* Esta função deve:
 * - Serializar a mensagem de resposta contida em msg;
 * - Libertar a memória ocupada por esta mensagem;
 * - Enviar a mensagem serializada, através do client_socket.
 */
int network_send(int client_socket, struct message_t *msg) {
    struct _MessageT msgSerial;
    message_t__init(&msgSerial);
    msgSerial.opcode = msg->message->opcode;
    msgSerial.c_type = msg->message->c_type;
    msgSerial.data_size = msg->message->data_size;
    msgSerial.key = msg->message->key;
    msgSerial.sizeheight = msg->message->sizeheight;
    msgSerial.keys = msg->message->keys;
    msgSerial.data = msg->message->data;
    msgSerial.op = msg->message->op;
    unsigned len = message_t__get_packed_size(&msgSerial);
    uint8_t *buf;
    buf = malloc(len);
    if(buf == NULL) {
        return -1;
    }
    message_t__pack(&msgSerial, buf);
    char bufsize[4];
    sprintf(bufsize, "%d", len);
    write_all(client_socket, bufsize, sizeof(bufsize));
    int result = write_all(client_socket, buf, len);

    free(buf);
    free(msg);
    return result;
}

/* A função network_server_close() liberta os recursos alocados por
 * network_server_init().
 */
int network_server_close() {
    free(&server);
    close(portS);
    return 0;
}

/* Esta função deve:
 * - Aceitar uma conexão de um cliente;
 * - Receber uma mensagem usando a função network_receive;
 * - Entregar a mensagem de-serializada ao skeleton para ser processada;
 * - Esperar a resposta do skeleton;
 * - Enviar a resposta ao cliente usando a função network_send.
 */
int network_main_loop(int listening_socket) {
    struct sockaddr_in client;
    socklen_t size_client;

    int statusOp = 0;
    for (int i = 0; i < NFDESC; i++) {
        connections[i].fd = -1;
    }
    connections[0].fd = listening_socket;
    connections[0].events = POLLIN;

    int nfds = 1;
    int kfds;

    while ((kfds = poll(connections, nfds, 10)) >= 0) {
        
        if(kfds > 0) {

            if((connections[0].revents & POLLIN) && (nfds < NFDESC)) {
                if((connections[nfds].fd = accept(connections[0].fd, (struct sockaddr *) &client, &size_client)) > 0) {
                    connections[nfds].events = POLLIN;
                    nfds++;
                }
            }
            
            for (int i = 1; i < NFDESC; i++) {
                if(connections[i].revents & POLLIN) {
                        struct message_t *requestResult;
                        requestResult = network_receive(connections[i].fd);
                        if(requestResult == NULL) {
                            printf("Client Quit\n");
                            close(connections[i].fd);
                            connections[i].fd = -1;
                            break;
                        }
                        statusOp = invoke(requestResult);
                        network_send(connections[i].fd, requestResult);
                    
		            // Fecha socket referente a esta conexão
                }
                if(connections[i].revents == POLLERR || connections[i].revents == POLLHUP) {
                    close(connections[i].fd);
                    connections[i].fd = -1;
                }
            }  
        }
    }
    network_server_close();
    return statusOp;
}