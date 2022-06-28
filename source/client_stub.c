/* 
Projecto de SD - Fase 4
Grupo: 45
António Pereira - 50320
Guilherme Lopes - 52761
Ricardo Banon - 42035
*/

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "client_stub_private.h"
#include "network_client.h"
#include "message-private.h"
#include "zookeeper/zookeeper.h"

#define ZDATALEN 1024 * 1024

struct rtree_t *rtree;
static int is_connected;
static zhandle_t * zh;
typedef struct String_vector zoo_string;
zoo_string* children_list;
static char *watcher_ctx = "ZooKeeper Child Watcher";
static char *kv_path = "/kvstore";
static char *prm_path = "/kvstore/primary";
static char *bck_path = "/kvstore/backup";

void connection_watcher(zhandle_t *zzh, int type, int state, const char*path, void* context) {
    if (type == ZOO_SESSION_EVENT) {
        if (state == ZOO_CONNECTED_STATE) {
            is_connected = 1;
        } else {
            is_connected = 0;
        }
    }
}

static void child_watcher(zhandle_t *zzh, int type, int state, const char*path, void* watcher_ctx) {
    zoo_string* children_list2 = (zoo_string *) malloc(sizeof(zoo_string));
    int zoo_data_len = ZDATALEN;
    if (state == ZOO_CONNECTED_STATE) {
        if(type == ZOO_CHILD_EVENT) {
            char *keyB = "backup";
            char *keyP = "primary";
            char *nc = "notconnected";

            if(ZOK != zoo_get_children(zh, kv_path, 0, children_list2)) {
                fprintf(stderr, "Error setting watch at %s!\n", kv_path);
            }

            if(children_list2->count == 0) {
                printf("0");
                struct data_t *data;
                data = data_create2(sizeof(char)*strlen(nc), nc); 
                rtree_put_data(rtree, keyP, data);
                rtree_put_data(rtree, keyB, data);
            }
            else if(children_list2->count == 1) {      
                int size = 1024*1024;
                struct data_t *dataB;
                struct data_t *dataP;

                char *ipPortBackup;
                ipPortBackup = (char *) malloc(1024);

                if(ZOK != zoo_get(zh, bck_path, 0, ipPortBackup, &size, NULL)) {
                    printf("no backup");
                    dataB = data_create2(sizeof(char)*strlen(nc), nc); 
                } else {
                    dataB = data_create2(sizeof(char)*strlen(ipPortBackup), ipPortBackup); 
                }
                rtree_put_data(rtree, keyB, dataB);

                char *ipPortPrimary;
                ipPortPrimary = (char *) malloc(1024);

                if(ZOK != zoo_get(zh, prm_path, 0, ipPortPrimary, &size, NULL)) {
                    printf("no primary");
                    dataP = data_create2(sizeof(char)*strlen(nc), nc); 
                } else {
                    dataP = data_create2(sizeof(char)*strlen(ipPortPrimary), ipPortPrimary);
                }
                rtree_put_data(rtree, keyP, dataP);

                int suspend = -1;
                network_susControl(suspend);              
            }
            else if (children_list2->count == 2) {
                int size = 1024*1024;
                struct data_t *dataB;
                struct data_t *dataP;

                char *ipPortBackup;
                ipPortBackup = (char *) malloc(1024);

                if(ZOK != zoo_get(zh, bck_path, 0, ipPortBackup, &size, NULL)) {
                    printf("no backup");
                    dataB = data_create2(sizeof(char)*strlen(nc), nc); 
                } else {
                    dataB = data_create2(sizeof(char)*strlen(ipPortBackup), ipPortBackup); 
                }
                rtree_put_data(rtree, keyB, dataB);

                char *ipPortPrimary;
                ipPortPrimary = (char *) malloc(1024);

                if(ZOK != zoo_get(zh, prm_path, 0, ipPortPrimary, &size, NULL)) {
                    printf("no primary");
                    dataP = data_create2(sizeof(char)*strlen(nc), nc); 
                } else {
                    dataP = data_create2(sizeof(char)*strlen(ipPortPrimary), ipPortPrimary);
                }
                rtree_put_data(rtree, keyP, dataP);
                
                int cont = 0;
                network_susControl(cont);      
                network_connect(rtree);
            }
            if(ZOK != zoo_wget_children(zh, kv_path, &child_watcher, watcher_ctx, children_list)) {
                fprintf(stderr, "Error setting watch at %s!\n", kv_path);
            }
        }
    }
}

/* Função para estabelecer uma associação entre o cliente e o servidor, 
 * em que address_port é uma string no formato <hostname>:<port>.
 * Retorna NULL em caso de erro.
 */
struct rtree_t *rtree_connect(const char *address_port) {
    if(address_port == NULL)
        return NULL;
    rtree = (struct rtree_t *) malloc (sizeof(struct rtree_t));
    if(rtree == NULL)
        return NULL;
    
    zh = zookeeper_init(address_port, connection_watcher, 2000, 0, NULL, 0);
    if(zh == NULL) {
        fprintf(stderr, "Error connecting to ZooKeeper Server[%d]!\n", errno);
        exit(EXIT_FAILURE);
    }
    
    children_list = (zoo_string *) malloc(sizeof(zoo_string));
    if(ZOK != zoo_wget_children(zh, kv_path, &child_watcher, watcher_ctx, children_list)) {
        fprintf(stderr, "Error setting watch at %s!\n", kv_path);
    }
    
    char *ipPortPrimary;
    ipPortPrimary = (char *) malloc(1024);
    char *ipPortBackup;
    ipPortBackup = (char *) malloc(1024);
    int size = 1024*1024;
    if(ZOK != zoo_get(zh, prm_path, 0, ipPortPrimary, &size, NULL)) {
        fprintf(stderr, "Error getting primary at %s!\n", kv_path);
    }

    if(ZOK != zoo_get(zh, bck_path, 0, ipPortBackup, &size, NULL)) {
        fprintf(stderr, "Error getting primary at %s!\n", kv_path);
    }

    char *keyP = "primary";
    char *keyB = "backup";
    struct data_t *dataP;
    dataP = data_create2(sizeof(char)*strlen(ipPortPrimary), ipPortPrimary); 
    struct data_t *dataB;
    dataB = data_create2(sizeof(char)*strlen(ipPortBackup), ipPortBackup); 
    rtree_put_data(rtree, keyP, dataP);
    rtree_put_data(rtree, keyB, dataB);
    if(network_connect(rtree) == -1) {
        return NULL;
    }
    return rtree;
}

/* Termina a associação entre o cliente e o servidor, fechando a 
 * ligação com o servidor e libertando toda a memória local.
 * Retorna 0 se tudo correr bem e -1 em caso de erro.
 */
int rtree_disconnect(struct rtree_t *rtree) {
    int res;
    if(rtree == NULL)
        return -1;
    if(rtree_getsize(rtree) == 0) {
        return 0;
    }
    res = network_close(rtree);
    if(res == 0) {
        res = rtree_destroy(rtree);
    }
    return res;
}

/* Função privada para apagar árvore
*/
int rtree_destroy(struct rtree_t *rtree) {
    if(rtree != NULL) {
        rtree_destroy(rtree->left);
        rtree_destroy(rtree->right);
        free(rtree->entry);
        return 0;
    }
    return -1;
}

int rtree_getsize(struct rtree_t *rtree) {
    if(rtree == NULL)
        return 0;
    else if (rtree->entry == NULL)
        return 0;

    return (rtree_getsize(rtree->left) + 1 + rtree_getsize(rtree->right));
}


/* Função para adicionar um elemento na árvore.
 * Se a key já existe, vai substituir essa entrada pelos novos dados.
 * Devolve 0 (ok, em adição/substituição) ou -1 (problemas).
 */
int rtree_put(struct rtree_t *rtree, struct entry_t *entry) {
    if(rtree == NULL || entry == NULL)
        return -1;

    struct message_t *msg_saida, *msg_resposta;
    msg_saida = (struct message_t *) malloc (sizeof(struct message_t));
    msg_saida->message = (struct _MessageT *) malloc (sizeof(struct _MessageT));
    if(msg_saida == NULL)
        return -1;
    msg_saida->message->opcode = MESSAGE_T__OPCODE__OP_PUT;
    msg_saida->message->c_type = MESSAGE_T__C_TYPE__CT_ENTRY;
    msg_saida->message->data_size = entry->value->datasize;
    msg_saida->message->data = (char *) entry->value->data;
    msg_saida->message->key = entry->key;

    msg_resposta = network_send_receive(rtree, msg_saida);


    int result = 0;
    if(msg_resposta == NULL) {
        result = -1;
    }
    if(msg_resposta->message->opcode != MESSAGE_T__OPCODE__OP_PUT + 1 && msg_resposta->message->c_type != MESSAGE_T__C_TYPE__CT_NONE) {
        result = -1;
    }
    result = msg_resposta->message->op;
    free(msg_resposta);
    return result;
}

/* Função para obter um elemento da árvore.
 * Em caso de erro, devolve NULL.
 */
struct data_t *rtree_get(struct rtree_t *rtree, char *key) {
    if(rtree == NULL)
        return NULL;
    if(key == NULL)
        return NULL;

    struct message_t *msg_saida, *msg_resposta;
    msg_saida = (struct message_t *) malloc (sizeof(struct message_t));
    msg_saida->message = (struct _MessageT *) malloc (sizeof(struct _MessageT));
    if(msg_saida == NULL)
        return NULL;
    
    msg_saida->message->opcode = MESSAGE_T__OPCODE__OP_GET;
    msg_saida->message->c_type = MESSAGE_T__C_TYPE__CT_KEY;
    msg_saida->message->key = key;

    msg_resposta = network_send_receive(rtree, msg_saida);
    free(msg_saida);

    if(msg_resposta == NULL) {
        return NULL;
    }
    if(msg_resposta->message->opcode != (MESSAGE_T__OPCODE__OP_GET + 1) && msg_resposta->message->c_type != MESSAGE_T__C_TYPE__CT_VALUE) {
        return NULL;
    }
    struct data_t *data;
    if(msg_resposta->message->data_size < 1 || msg_resposta->message->data == NULL) {
        data = (struct data_t *) malloc (sizeof(struct data_t));
        data->datasize = msg_resposta->message->data_size;
        data->data = NULL;
        return data;
    }
    data = data_create2(msg_resposta->message->data_size, strdup(msg_resposta->message->data));
    free(msg_resposta);
    return data;
}

struct data_t *rtree_get_data(struct rtree_t *rtree, char *key) {
    if(rtree == NULL)
        return NULL;

    if(key == NULL || rtree->entry== NULL)
        return NULL;

    if (strcmp(rtree->entry->key, key) == 0)
        return data_dup(rtree->entry->value);
    else if (strcmp(rtree->entry->key, key) < 0)
        return rtree_get_data(rtree->right, key);
    else if (strcmp(rtree->entry->key, key) > 0)
        return rtree_get_data(rtree->left, key);

    return NULL;
}


int rtree_put_data(struct rtree_t *rtree, char *key, struct data_t *value) {
    if(rtree == NULL || key == NULL || value == NULL)
        return -1;

    char *tempKey = strdup(key);

    struct data_t *tempVal;
    tempVal = data_dup(value);

    struct rtree_t *root;
    root = rtree;
    
    struct entry_t *tempEntry;
    tempEntry = entry_create(tempKey, tempVal);

    rtree_put_aux(rtree, tempEntry);
    
    if(rtree == NULL)
        return -1;

    rtree = root;
    
    return 0;
}

struct rtree_t *rtree_put_aux(struct rtree_t *rtree, struct entry_t *tempEntry) {
    if(rtree == NULL) {
        struct rtree_t *rtree;
        rtree = (struct rtree_t *) malloc (sizeof(struct rtree_t));

        if(rtree == NULL)
            return NULL;

        
        rtree->entry = entry_dup(tempEntry);
        entry_destroy(tempEntry);
        return rtree;
    }

    if(rtree->entry == NULL) {
        entry_destroy(rtree->entry);
        rtree->entry = entry_dup(tempEntry);
        entry_destroy(tempEntry);
        return rtree;
    }

    int result = entry_compare(rtree->entry, tempEntry);
    if(result == 0) {
        char *newKey = strdup(tempEntry->key);
        struct data_t *newVal = data_dup(tempEntry->value);
        entry_replace(rtree->entry, newKey, newVal);
        entry_destroy(tempEntry);
        return rtree;
    } else if (result > 0) {
        rtree->left = rtree_put_aux(rtree->left, tempEntry);
    } else if (result < 0) {
        rtree->right = rtree_put_aux(rtree->right, tempEntry);
    }

    return rtree;
}
/* Função para remover um elemento da árvore. Vai libertar 
 * toda a memoria alocada na respetiva operação rtree_put().
 * Devolve: 0 (ok), -1 (key not found ou problemas).
 */
int rtree_del(struct rtree_t *rtree, char *key) {
    if(rtree == NULL)
        return -1;
    if(key == NULL)
        return -1;

    struct message_t *msg_saida, *msg_resposta;
    msg_saida = (struct message_t *) malloc (sizeof(struct message_t));
    if(msg_saida == NULL)
        return -1;
    
    msg_saida->message->opcode = MESSAGE_T__OPCODE__OP_DEL;
    msg_saida->message->c_type = MESSAGE_T__C_TYPE__CT_KEY;
    msg_saida->message->key = key;

    msg_resposta = network_send_receive(rtree, msg_saida);
    free(msg_saida);

    int result = 0;
    if(msg_resposta == NULL)
        return -1;
    if(msg_resposta->message->opcode != MESSAGE_T__OPCODE__OP_DEL + 1 && msg_resposta->message->c_type != MESSAGE_T__C_TYPE__CT_VALUE) {
        msg_resposta->message->opcode = MESSAGE_T__OPCODE__OP_ERROR;
        msg_resposta->message->c_type = MESSAGE_T__C_TYPE__CT_NONE;
        result = -1;
    }
    result = msg_resposta->message->op;
    free(msg_resposta);
    return result;
}

/* Devolve o número de elementos contidos na árvore.
 */
int rtree_size(struct rtree_t *rtree) {
    if(rtree == NULL)
        return 0;

    struct message_t *msg_saida, *msg_resposta;
    msg_saida = (struct message_t *) malloc (sizeof(struct message_t));
    if(msg_saida == NULL)
        return 0;
    
    msg_saida->message->opcode = MESSAGE_T__OPCODE__OP_SIZE;
    msg_saida->message->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    msg_resposta = network_send_receive(rtree, msg_saida);
    free(msg_saida);
    
    if(msg_resposta == NULL)
        return -1;

    int result = msg_resposta->message->sizeheight;
    free(msg_resposta);
    return result;
}

/* Função que devolve a altura da árvore.
 */
int rtree_height(struct rtree_t *rtree) {
    if(rtree == NULL)
        return 0;

    struct message_t *msg_saida, *msg_resposta;
    msg_saida = (struct message_t *) malloc (sizeof(struct message_t));
    if(msg_saida == NULL)
        return 0;
    
    msg_saida->message->opcode = MESSAGE_T__OPCODE__OP_HEIGHT;
    msg_saida->message->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    msg_resposta = network_send_receive(rtree, msg_saida);
    free(msg_saida);
    
    if(msg_resposta == NULL)
        return -1;

    int result = msg_resposta->message->sizeheight;
    free(msg_resposta);
    return result;
}

/* Devolve um array de char* com a cópia de todas as keys da árvore,
 * colocando um último elemento a NULL.
 */
char **rtree_get_keys(struct rtree_t *rtree) {
    if(rtree == NULL)
        return NULL;

    struct message_t *msg_saida, *msg_resposta;
    msg_saida = (struct message_t *) malloc (sizeof(struct message_t));
    if(msg_saida == NULL)
        return NULL;
    
    msg_saida->message->opcode = MESSAGE_T__OPCODE__OP_GETKEYS;
    msg_saida->message->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    msg_resposta = network_send_receive(rtree, msg_saida);
    free(msg_saida);
    
    if(msg_resposta == NULL)
        return NULL;

    char **array_keys =  msg_resposta->message->keys;
    free(msg_resposta);
    return array_keys;
}

/* Liberta a memória alocada por rtree_get_keys().
 */
void rtree_free_keys(char **keys) {
    for(int i = 0; keys[i] != NULL; i++) {
        free(keys[i]);
    }
    free(keys);
}

int rtree_verify(struct rtree_t *rtree,int op_n) {
    if(rtree == NULL)
        return -1;

    struct message_t *msg_saida, *msg_resposta;
    msg_saida = (struct message_t *) malloc (sizeof(struct message_t));
    if(msg_saida == NULL)
        return -1;
    
    msg_saida->message->opcode = MESSAGE_T__OPCODE__OP_VERIFY;
    msg_saida->message->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
    msg_saida->message->op = op_n;

    msg_resposta = network_send_receive(rtree, msg_saida);
    free(msg_saida);
    
    if(msg_resposta == NULL)
        return -1;

    int result =  msg_resposta->message->op;
    free(msg_resposta);
    return result;
}