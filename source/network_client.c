/* 
Projecto de SD - Fase 4
Grupo: 45
António Pereira - 50320
Guilherme Lopes - 52761
Ricardo Banon - 42035
*/

#include "network_client.h"
#include "client_stub_private.h"
#include "inet.h"
#include "message-private.h"
#include <stdio.h>
#include <string.h>

int sendStatus = 0;
/* Esta função deve:
 * - Obter o endereço do servidor (struct sockaddr_in) a base da
 *   informação guardada na estrutura rtree;
 * - Estabelecer a ligação com o servidor;
 * - Guardar toda a informação necessária (e.g., descritor do socket)
 *   na estrutura rtree;
 * - Retornar 0 (OK) ou -1 (erro).
 */
int network_connect(struct rtree_t *rtree) {
    if(rtree == NULL) {
        printf("rtree null netcon\n");
        return -1; 
    }

    int sockfdP;
    struct sockaddr_in serverP;
    int sockfdB;
    struct sockaddr_in serverB;
    struct data_t *dataP;
    struct data_t *dataB;
    char *keyPrm = "primary";
    char *keyBck = "backup";
    dataP = rtree_get_data(rtree, keyPrm);
    dataB = rtree_get_data(rtree, keyBck);

    if(dataP != NULL) {
        //collecting the adress
        char *d = (char *)dataP->data;
        char *token = strtok(d, ":");
        char *ipP = token;
        token = strtok(NULL, " ");
        char *portP = token;
        
        // Creates socket TCP
        if((sockfdP = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Error: creating socket TCP");
            return -1;
        }

        // Fills the structer server with the adress of the server to stablish
        //connection
        serverP.sin_family = AF_INET;
        serverP.sin_port = htons(atoi(portP));
        if(inet_pton(AF_INET, ipP, &serverP.sin_addr) < 1) {
            printf("Error: Converting IP\n");
            close(sockfdP); //closes socket
            return -1;
        }
        // Stablish connection with the server, defined on the structer of the server
        if(connect(sockfdP,(struct sockaddr *)&serverP, sizeof(serverP)) < 0) {
            perror("Error: connecting to server");
            close(sockfdP); //closes socket
            return -1;
        }
        
        struct data_t *data2P;
        data2P = data_create2(sizeof(int), &sockfdP);
        char *key2P = "primary2";
        rtree_put_data(rtree, key2P, data2P);     
    }

    if(dataB != NULL) {
        //collecting the adress
        char *d = (char *)dataB->data;
        char *token = strtok(d, ":");
        char *ipB = token;
        token = strtok(NULL, " ");
        char *portB = token;
        
        // Creates socket TCP
        if((sockfdB = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Error: creating socket TCP");
            return -1;
        }

        // Fills the structer server with the adress of the server to stablish
        //connection
        serverB.sin_family = AF_INET;
        serverB.sin_port = htons(atoi(portB));
        if(inet_pton(AF_INET, ipB, &serverB.sin_addr) < 1) {
            printf("Error: Converting IP\n");
            close(sockfdB); //closes socket
            return -1;
        }

        // Stablish connection with the server, defined on the structer of the server
        if(connect(sockfdB,(struct sockaddr *)&serverB, sizeof(serverB)) < 0) {
            perror("Error: connecting to server");
            close(sockfdB); //closes socket
            return -1;
        }
        
        struct data_t *data2B;
        data2B = data_create2(sizeof(int), &sockfdB);
        char *key2B = "backup2";
        rtree_put_data(rtree, key2B, data2B);    
    }   

    return 0; //End
}

/* Esta função deve:
 * - Obter o descritor da ligação (socket) da estrutura rtree_t;
 * - Serializar a mensagem contida em msg;
 * - Enviar a mensagem serializada para o servidor;
 * - Esperar a resposta do servidor;
 * - De-serializar a mensagem de resposta;
 * - Retornar a mensagem de-serializada ou NULL em caso de erro.
 */
struct message_t *network_send_receive(struct rtree_t * rtree, struct message_t *msg) {
    if(sendStatus == -1) {
        return msg;
    }
    struct message_t *msg_resposta;
    msg_resposta = (struct message_t *) malloc (sizeof(struct message_t));
    msg_resposta->message = (struct _MessageT *) malloc (sizeof(struct _MessageT));
    
    uint8_t *buf = NULL;
    uint8_t *buf2 = NULL;
    if(rtree == NULL || msg == NULL) {
        return NULL;
    }

    struct data_t * socketInfoP;
    char *socketKeyP;
    socketKeyP = "primary2";
    socketInfoP = rtree_get_data(rtree, socketKeyP);
    if(socketInfoP == NULL) {
        return NULL;
    }
    int sockfP = *(int *) socketInfoP->data;

    struct data_t * socketInfoB;
    char *socketKeyB;
    socketKeyB = "backup2";
    socketInfoB = rtree_get_data(rtree, socketKeyB);
    if(socketInfoB == NULL) {
        return NULL;
    }
    int sockfB = *(int *) socketInfoB->data;

    //MSG
    unsigned len;
    struct _MessageT msgSerial;
    message_t__init(&msgSerial);
    msgSerial.opcode = msg->message->opcode;
    msgSerial.c_type = msg->message->c_type;
    msgSerial.data_size = msg->message->data_size;
    msgSerial.key = msg->message->key;
    msgSerial.sizeheight = msg->message->sizeheight;
    msgSerial.keys = msg->message->keys;
    msgSerial.data = (char *) msg->message->data;
    msgSerial.op = msg->message->op;
    len = message_t__get_packed_size(&msgSerial);
    buf = malloc(len);
    if(buf == NULL) {
        return NULL;
    }

    message_t__pack(&msgSerial, buf);

    char bufsize[4];
    sprintf(bufsize, "%d", len);

    //write and read
    if(msg->message->opcode == MESSAGE_T__OPCODE__OP_PUT || msg->message->opcode == MESSAGE_T__OPCODE__OP_DEL) {
        write_all(sockfP, bufsize, sizeof(bufsize));

        write_all(sockfP, buf, len);

        int packSize2;
        char size2[4];
        packSize2 = read_all(sockfP, size2, sizeof(size2));

        packSize2 = atoi(size2);
        buf2 = malloc(packSize2);

        read_all(sockfP, buf2, packSize2);

        struct _MessageT *msgRcv = NULL;

        msgRcv = message_t__unpack(NULL, packSize2, buf2);

        msg_resposta->message->opcode = msgRcv->opcode;
        msg_resposta->message->c_type = msgRcv->c_type;
        msg_resposta->message->data_size = msgRcv->data_size;
        msg_resposta->message->key = strdup(msgRcv->key);
        msg_resposta->message->sizeheight = msgRcv->sizeheight;
        msg_resposta->message->keys = msgRcv->keys;
        msg_resposta->message->data = strdup(msgRcv->data);
        msg_resposta->message->op = msgRcv->op;
        message_t__free_unpacked(msgRcv, NULL);
    } else {
        write_all(sockfB, bufsize, sizeof(bufsize));

        write_all(sockfB, buf, len);

        int packSize2;
        char size2[4];
        packSize2 = read_all(sockfB, size2, sizeof(size2));

        packSize2 = atoi(size2);
        buf2 = malloc(packSize2);

        read_all(sockfB, buf2, packSize2);

        struct _MessageT *msgRcv = NULL;

        msgRcv = message_t__unpack(NULL, packSize2, buf2);

        msg_resposta->message->opcode = msgRcv->opcode;
        msg_resposta->message->c_type = msgRcv->c_type;
        msg_resposta->message->data_size = msgRcv->data_size;
        msg_resposta->message->key = strdup(msgRcv->key);
        msg_resposta->message->sizeheight = msgRcv->sizeheight;
        msg_resposta->message->keys = msgRcv->keys;
        msg_resposta->message->data = strdup(msgRcv->data);
        msg_resposta->message->op = msgRcv->op;
        message_t__free_unpacked(msgRcv, NULL);
    }
    return msg_resposta;
}

/* A função network_close() fecha a ligação estabelecida por
 * network_connect().
 */
int network_close(struct rtree_t *rtree) {
    char *keyP = "primary2";
    struct data_t *sockeP;
    sockeP = rtree_get_data(rtree, keyP);
    char *keyB = "backup2";
    struct data_t *sockeB;
    sockeB = rtree_get_data(rtree, keyB);
    int sockCloseP = *(int *) sockeP->data;
    int sockCloseB = *(int *) sockeB->data;
    char *quitmsg = "quit";
    write_all(sockCloseP, quitmsg, sizeof(quitmsg));
    write_all(sockCloseB, quitmsg, sizeof(quitmsg));
    close(sockCloseP);
    close(sockCloseB);
    return 0;
}

struct message_t *network_send_backup(struct rtree_t * rtree, struct message_t *msg) {
    if(sendStatus == -1) {
        printf("sendStatus\n");
        return msg;
    }
    struct message_t *msg_resposta;
    msg_resposta = (struct message_t *) malloc (sizeof(struct message_t));
    msg_resposta->message = (struct _MessageT *) malloc (sizeof(struct _MessageT));
    
    uint8_t *buf = NULL;
    uint8_t *buf2 = NULL;
    if(rtree == NULL) {
        printf("rtree null\n");
        return NULL;
    }
    if(msg == NULL) {
        printf("msg null\n");
        return NULL;
    }

    struct data_t * socketInfoB;
    char *socketKeyB;
    socketKeyB = "backup2";
    socketInfoB = rtree_get_data(rtree, socketKeyB);
    if(socketInfoB == NULL) {
        printf("socketinfo null\n");
        return NULL;
    }
    int sockfB = *(int *) socketInfoB->data;

    //MSG
    unsigned len;
    struct _MessageT msgSerial;
    message_t__init(&msgSerial);
    msgSerial.opcode = msg->message->opcode;
    msgSerial.c_type = msg->message->c_type;
    msgSerial.data_size = msg->message->data_size;
    msgSerial.key = msg->message->key;
    msgSerial.sizeheight = msg->message->sizeheight;
    msgSerial.keys = msg->message->keys;
    msgSerial.data = (char *) msg->message->data;
    msgSerial.op = msg->message->op;
    len = message_t__get_packed_size(&msgSerial);
    buf = malloc(len);
    if(buf == NULL) {
        printf("buf null\n");
        return NULL;
    }

    message_t__pack(&msgSerial, buf);

    char bufsize[4];
    sprintf(bufsize, "%d", len);

    //write and read
    if(msg->message->opcode == MESSAGE_T__OPCODE__OP_PUT || msg->message->opcode == MESSAGE_T__OPCODE__OP_DEL) {
        write_all(sockfB, bufsize, sizeof(bufsize));
        write_all(sockfB, buf, len);

        int packSize2;
        char size2[4];
        packSize2 = read_all(sockfB, size2, sizeof(size2));

        packSize2 = atoi(size2);
        buf2 = malloc(packSize2);

        read_all(sockfB, buf2, packSize2);

        struct _MessageT *msgRcv = NULL;

        msgRcv = message_t__unpack(NULL, packSize2, buf2);

        msg_resposta->message->opcode = msgRcv->opcode;
        msg_resposta->message->c_type = msgRcv->c_type;
        msg_resposta->message->data_size = msgRcv->data_size;
        msg_resposta->message->key = strdup(msgRcv->key);
        msg_resposta->message->sizeheight = msgRcv->sizeheight;
        msg_resposta->message->keys = msgRcv->keys;
        msg_resposta->message->data = strdup(msgRcv->data);
        msg_resposta->message->op = msgRcv->op;
        message_t__free_unpacked(msgRcv, NULL);
        return msg_resposta;
    } 
    return NULL;
}

int network_connecttobck(struct rtree_t *rtree) {
    if(rtree == NULL) {
        printf("rtree null netcon\n");
        return -1; 
    }

    int sockfdB;
    struct sockaddr_in serverB;
    struct data_t *dataB;
    char *keyBck = "backup";
    dataB = rtree_get_data(rtree, keyBck);

    if(dataB != NULL) {
        //collecting the adress
        char *d = (char *)dataB->data;
        char *token = strtok(d, ":");
        char *ipB = token;
        token = strtok(NULL, " ");
        char *portB = token;
        
        // Creates socket TCP
        if((sockfdB = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Error: creating socket TCP");
            return -1;
        }

        // Fills the structer server with the adress of the server to stablish
        //connection
        serverB.sin_family = AF_INET;
        serverB.sin_port = htons(atoi(portB));
        if(inet_pton(AF_INET, ipB, &serverB.sin_addr) < 1) {
            printf("Error: Converting IP\n");
            close(sockfdB); //closes socket
            return -1;
        }

        // Stablish connection with the server, defined on the structer of the server
        if(connect(sockfdB,(struct sockaddr *)&serverB, sizeof(serverB)) < 0) {
            perror("Error: connecting to server");
            close(sockfdB); //closes socket
            return -1;
        }
        
        struct data_t *data2B;
        data2B = data_create2(sizeof(int), &sockfdB);
        char *key2B = "backup2";
        rtree_put_data(rtree, key2B, data2B);    
    }   

    return 0; //End
}


int network_susControl(int x) {
    sendStatus = x;
    return 0;
}
