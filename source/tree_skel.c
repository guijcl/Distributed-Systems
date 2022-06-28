/* 
Projecto de SD - Fase 4
Grupo: 45
António Pereira - 50320
Guilherme Lopes - 52761
Ricardo Banon - 42035
*/

#include "tree_skel.h"
#include "task.h"
#include "client_stub_private.h"
#include "network_client.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <netdb.h>
#include "inet.h"
#include "zookeeper/zookeeper.h"

#define ZDATALEN 1024 * 1024

struct tree_t *tree_global;
struct task_t *queue_head;

struct rtree_t *zoo_tree;
char * zooIpAndPort;
char * serverPort;
char * serverIPPort;
char * pserverIPPort;
static char *kv_path = "/kvstore";
static char *prm_path = "/kvstore/primary";
static char *bck_path = "/kvstore/backup";
char *role;
static int is_connected;
typedef struct String_vector zoo_string;
static zhandle_t * zh;
zoo_string *children_list;
static char *watcher_ctx = "ZooKeeper Child Watcher";
int backupAvailable = -1;

pthread_t new;
pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t tree_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;

int last_assigned;
int op_count;

void connection_watcher_server(zhandle_t *zzh, int type, int state, const char*path, void* context) {
    if (type == ZOO_SESSION_EVENT) {
        if (state == ZOO_CONNECTED_STATE) {
            is_connected = 1;
        } else {
            is_connected = 0;
        }
    }
}

static void child_watcher_server(zhandle_t *zzh, int type, int state, const char*path, void* watcher_ctx) {
    zoo_string* children_list2 = (zoo_string *) malloc(sizeof(zoo_string));
    if (state == ZOO_CONNECTED_STATE) {
        if(type == ZOO_CHILD_EVENT) {
            if(ZOK != zoo_get_children(zh, kv_path, 0, children_list2)) {
                fprintf(stderr, "Error setting watch at %s!\n", kv_path);
            }
            if(strcmp(role, "primary") == 0) {
                if(children_list2->count == 1) {
                    backupAvailable = -1;
                }
                if(children_list2->count == 2) {
                    backupAvailable = 0;
                    pserverIPPort = (char *) malloc(1024);
                    int size = 1024*1024;
                    if(ZOK != zoo_get(zh, bck_path, 0, pserverIPPort, &size, NULL)) {
                        fprintf(stderr, "Error getting backup at %s!\n", bck_path);
                    }

                    char * keyB;
                    keyB = "backup";
                    struct data_t *dataB;
                    dataB = data_create2(sizeof(char)*strlen(pserverIPPort), pserverIPPort);
                    rtree_put_data(zoo_tree, keyB, dataB);
                    network_connecttobck(zoo_tree);
                }
            }
            if(strcmp(role, "backup") == 0 && children_list2->count == 1) {
                backupAvailable = -1;
                role = "primary";
                zoo_delete(zh, bck_path, -1);
                int lenServer;
                lenServer = strlen(serverIPPort);
                int new_path_len = 1024;
                char *newprm_path = malloc (new_path_len);
                if(ZOK != zoo_create(zh, prm_path, serverIPPort, lenServer, & ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, newprm_path, new_path_len)) {
                    fprintf(stderr, "Error creating znode from path %s!\n", prm_path);
                    exit(EXIT_FAILURE);
                }      
            }

            if(ZOK != zoo_wget_children(zh, kv_path, &child_watcher_server, watcher_ctx, children_list)) {
                fprintf(stderr, "Error setting watch at %s!\n", kv_path);
            }
        }
    }
}


/* Inicia o skeleton da árvore.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). 
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */
int tree_skel_init() {
    zoo_tree = (struct rtree_t *) malloc (sizeof(struct rtree_t));
    if(zoo_tree == NULL)
        return -1;

    //zookeeper
    zh = zookeeper_init(zooIpAndPort, connection_watcher_server, 2000, 0, NULL, 0);
    if(zh == NULL) {
        fprintf(stderr, "Error connecting to ZooKeeper Server[%d]!\n", errno);
        exit(EXIT_FAILURE);
    }

    char hostbuffer[256];
    char * dpontos;
    dpontos = ":";
    int hostname;
    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    char *IPbuffer;
    struct hostent *host_entry;
    host_entry = gethostbyname(hostbuffer);
    IPbuffer = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
    serverIPPort = (char *) malloc (strlen(IPbuffer) + 1 + strlen(serverPort) + 1);
    pserverIPPort = (char *) malloc (strlen(IPbuffer) + 1 + strlen(serverPort) + 1);
    strcpy(serverIPPort, IPbuffer);
    strcat(serverIPPort, dpontos);
    strcat(serverIPPort, serverPort);
    int lenServer;
    lenServer = strlen(serverIPPort);
    sleep(3);
    if (is_connected) {
        int new_path_len = 1024;
        char *new_path = malloc (new_path_len);
        char *newprm_path = malloc (new_path_len);
        char *newbck_path = malloc (new_path_len);
        if (ZNONODE == zoo_exists(zh, kv_path, 0, NULL)) {
            if(ZOK != zoo_create(zh, kv_path, serverIPPort, lenServer, & ZOO_OPEN_ACL_UNSAFE, 0, new_path, new_path_len)) {
                fprintf(stderr, "Error creating znode from path %s!\n", kv_path);
                exit(EXIT_FAILURE);
            }
            fprintf(stderr, "Normal ZNode created! ZNode path: %s\n", new_path);
            if(ZOK != zoo_create(zh, prm_path, serverIPPort, lenServer, & ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, newprm_path, new_path_len)) {
                fprintf(stderr, "Error creating znode from path %s!\n", prm_path);
                exit(EXIT_FAILURE);
            }
            role = "primary";
            fprintf(stderr, "Ephemeral ZNode created! ZNode path: %s\n", newprm_path);
        } else if (ZOK == zoo_exists(zh, kv_path, 0, NULL)) {
            int retval;
            zoo_string * child_list = (zoo_string *) malloc(sizeof(zoo_string));
            retval = zoo_get_children(zh, kv_path, 0, child_list);
            if (retval != ZOK) {
                fprintf(stderr, "Error retrieving znode from path %s!\n", kv_path);
                exit(EXIT_FAILURE);
            }
            if(child_list->count == 0) {
                if(ZOK != zoo_create(zh, prm_path, serverIPPort, lenServer, & ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, newprm_path, new_path_len)) {
                    fprintf(stderr, "Error creating znode from path %s!\n", prm_path);
                    exit(EXIT_FAILURE);
                }
                role = "primary";
                fprintf(stderr, "Ephemeral ZNode created! ZNode path: %s\n", newprm_path);
            } 
            if(child_list->count == 1) {
                if(ZOK == zoo_exists(zh, prm_path, 0, NULL)) {
                   if(ZOK != zoo_create(zh, bck_path, serverIPPort, lenServer, & ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, newbck_path, new_path_len)) {
                        fprintf(stderr, "Error creating znode from path %s!\n", bck_path);
                        exit(EXIT_FAILURE);
                    }
                    role = "backup";
                    fprintf(stderr, "Ephemeral ZNode created! ZNode path: %s\n", newbck_path);
                }
            }
            free(child_list);
        }

        children_list = (zoo_string *) malloc(sizeof(zoo_string));
        if(ZOK != zoo_wget_children(zh, kv_path, &child_watcher_server, watcher_ctx, children_list)) {
            fprintf(stderr, "Error setting watch at %s!\n", kv_path);
        }
        
        if(children_list->count == 2) {
            backupAvailable = 0;
            pserverIPPort = (char *) malloc(1024);
            int size = 1024*1024;

            char * keyB;
            if (strcmp(role, "primary") == 0) {
                keyB = "backup";
                if(ZOK != zoo_get(zh, bck_path, 0, pserverIPPort, &size, NULL)) {
                    fprintf(stderr, "Error getting backup at %s!\n", bck_path);
                }
            } else {
                keyB = "primary";
                if(ZOK != zoo_get(zh, prm_path, 0, pserverIPPort, &size, NULL)) {
                    fprintf(stderr, "Error getting primary at %s!\n", prm_path);
                }
            }
            struct data_t *dataB;
            dataB = data_create2(sizeof(char)*strlen(pserverIPPort),pserverIPPort);
            rtree_put_data(zoo_tree, keyB, dataB);  
        }
    }   


    //tree
    tree_global = tree_create();
    if(tree_global == NULL)
        return -1;
    //threading
    queue_head = NULL;
    last_assigned = 0;
    op_count = 0;
    if(pthread_create(&new, NULL, &process_task, NULL) != 0) {
        perror("\nThread not created \n");
        exit(EXIT_FAILURE);
    }
    if(pthread_detach(new)) {
        perror("\nThread not created \n");
        exit(EXIT_FAILURE);
    }
    return 0;
}

/* Liberta toda a memória e recursos alocados pela função tree_skel_init.
 */
void tree_skel_destroy() {
    tree_destroy(tree_global);
}

/* Executa uma operação na árvore (indicada pelo opcode contido em msg)
 * e utiliza a mesma estrutura message_t para devolver o resultado.
 * Retorna 0 (OK) ou -1 (erro, por exemplo, árvore nao incializada)
*/
int invoke(struct message_t *msg) {

    if(backupAvailable == -1) {
        printf("backup not up");
        msg->message->opcode = MESSAGE_T__OPCODE__OP_NOBCK;
        return 0;
    }

    int res;
    struct data_t *data_result;
    char* key_aux;

    if(msg == NULL)
        return -1;
    
    if((msg->message->opcode < 0 || msg->message->opcode > MESSAGE_T__OPCODE__OP_ERROR) ||
       (msg->message->c_type < 0 || msg->message->c_type > MESSAGE_T__C_TYPE__CT_NONE))
        return -1;
    
    if(msg->message->opcode == MESSAGE_T__OPCODE__OP_SIZE && msg->message->c_type == MESSAGE_T__C_TYPE__CT_NONE) {
        printf("Getting Size\n");
        res = tree_size(tree_global);
        msg->message->opcode = MESSAGE_T__OPCODE__OP_SIZE+1;
        msg->message->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        msg->message->sizeheight = res;
        return 0;
    }

    if(msg->message->opcode == MESSAGE_T__OPCODE__OP_HEIGHT && msg->message->c_type == MESSAGE_T__C_TYPE__CT_NONE) {
        printf("Getting Height\n");
        res = tree_height(tree_global);
        msg->message->opcode = MESSAGE_T__OPCODE__OP_HEIGHT+1;
        msg->message->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        msg->message->sizeheight = res;
        return 0;
    }

    if(msg->message->opcode == MESSAGE_T__OPCODE__OP_DEL && msg->message->c_type == MESSAGE_T__C_TYPE__CT_KEY) {
        printf("Deleting Entry\n");
        key_aux = strdup(msg->message->key);
        msg->message->op = last_assigned;
        struct task_t *taskDel = task_create(0, last_assigned, NULL, key_aux, NULL);
        queue_add_task(taskDel);
        last_assigned+=1;
        printf("Delete Successful\n");
        msg->message->opcode = MESSAGE_T__OPCODE__OP_DEL+1;
        msg->message->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        return 0;
    }

    if(msg->message->opcode == MESSAGE_T__OPCODE__OP_GET && msg->message->c_type == MESSAGE_T__C_TYPE__CT_KEY) {
        printf("Getting Data\n");
        key_aux = strdup(msg->message->key);
        data_result = tree_get(tree_global, key_aux);
        if(data_result == NULL) {
            printf("Data Not Found\n");
            msg->message->opcode = MESSAGE_T__OPCODE__OP_GET+1;
            msg->message->c_type = MESSAGE_T__C_TYPE__CT_VALUE;
            msg->message->data_size = 0;
            msg->message->data = NULL;
            return 0;
        }
        printf("Data Found Get Successful\n");
        msg->message->opcode = MESSAGE_T__OPCODE__OP_GET+1;
        msg->message->c_type = MESSAGE_T__C_TYPE__CT_VALUE;
        msg->message->data_size = data_result->datasize;
        msg->message->data = strdup(data_result->data);
        data_destroy(data_result);
        return 0;
    }

    if(msg->message->opcode == MESSAGE_T__OPCODE__OP_PUT && msg->message->c_type == MESSAGE_T__C_TYPE__CT_ENTRY) {
        printf("Putting Entry\n");
        key_aux = strdup(msg->message->key);
        char *data = strdup(msg->message->data);
        msg->message->op = last_assigned;
        struct task_t *taskPut = task_create(1, last_assigned, data, key_aux, NULL);
        queue_add_task(taskPut);
        last_assigned+=1;
        printf("Put Successful\n");
        msg->message->opcode = MESSAGE_T__OPCODE__OP_PUT+1;
        msg->message->c_type = MESSAGE_T__C_TYPE__CT_VALUE;
        return 0;
    }

    if(msg->message->opcode == MESSAGE_T__OPCODE__OP_GETKEYS && msg->message->c_type == MESSAGE_T__C_TYPE__CT_NONE) {
        printf("Getting Keys\n");
        char **keys;
        keys = tree_get_keys(tree_global);
        msg->message->opcode = MESSAGE_T__OPCODE__OP_GETKEYS+1;
        msg->message->c_type = MESSAGE_T__C_TYPE__CT_KEYS;
        msg->message->keys = keys;
        tree_free_keys(keys);
        return 0;
    }

    if(msg->message->opcode == MESSAGE_T__OPCODE__OP_VERIFY && msg->message->c_type == MESSAGE_T__C_TYPE__CT_RESULT) {
        printf("Checking If Executed\n");
        res = verify(msg->message->op);
        if(res == -1) {
            printf("Verify Error\n");
            msg->message->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            msg->message->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            return -1;
        }
        msg->message->opcode = MESSAGE_T__OPCODE__OP_VERIFY+1;
        msg->message->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        msg->message->op = res;
        return 0;
    }

    msg->message->opcode = MESSAGE_T__OPCODE__OP_ERROR;
    msg->message->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    return -1;
}

/* Verifica se a operação identificada por op_nfoi executada.
*/
int verify(int op_n){
    if (op_n >= last_assigned) {
        printf("verify not done");
        return -1;
    }
    printf("verify done");
    printf("\n%d \n",op_count);
    printf("\n%d \n",last_assigned);
    return op_count>op_n;
}

/*Função do thread secundário que vai processar pedidos de escrita.
*/
void * process_task(void * params) {
    struct thread_parameters *tp = (struct thread_parameters *) params;
    int res = 0;
    while(1) {
        struct task_t * nextTask = queue_get_task();
        pthread_mutex_lock(&queue_lock);
        pthread_mutex_lock(&tree_lock);
        while(op_count > last_assigned) {
            printf("waiting\n");
            pthread_cond_wait(&queue_not_empty, &queue_lock);
        }      
        if(nextTask->op == 0) {
            res = tree_del(tree_global, nextTask->key);
            printf("Deleting... \n");

            if(strcmp(role, "primary") == 0) {
                printf("Sending to backup... \n");
                struct message_t *msg_saida, *msg_resposta;
                msg_saida = (struct message_t *) malloc (sizeof(struct message_t));
                msg_saida->message = (struct _MessageT *) malloc (sizeof(struct _MessageT));
                if(msg_saida == NULL)
                    return NULL;
                msg_saida->message->opcode = MESSAGE_T__OPCODE__OP_DEL;
                msg_saida->message->c_type = MESSAGE_T__C_TYPE__CT_KEY;
                msg_saida->message->key = nextTask->key;

                msg_resposta = network_send_backup(zoo_tree, msg_saida);
            }
            op_count+=1;
        }
        if(nextTask->op == 1) {
            struct data_t* value;
            value = data_create2(sizeof(nextTask->data), nextTask->data);
            res = tree_put(tree_global, nextTask->key, value);
            printf("Putting... \n");

            if(strcmp(role, "primary") == 0) {
                printf("Sending to backup... \n");
                struct message_t *msg_saida, *msg_resposta;
                msg_saida = (struct message_t *) malloc (sizeof(struct message_t));
                msg_saida->message = (struct _MessageT *) malloc (sizeof(struct _MessageT));
                if(msg_saida == NULL)
                    return NULL;
                msg_saida->message->opcode = MESSAGE_T__OPCODE__OP_PUT;
                msg_saida->message->c_type = MESSAGE_T__C_TYPE__CT_ENTRY;
                msg_saida->message->data_size = sizeof(nextTask->data);
                msg_saida->message->data = nextTask->data;
                msg_saida->message->key = nextTask->key;

                msg_resposta = network_send_backup(zoo_tree, msg_saida);
            }
            op_count+=1;
        }
        pthread_mutex_unlock(&tree_lock);
        pthread_mutex_unlock(&queue_lock);
    }
    return &res; 
}

struct task_t *task_create(int op, int op_n, char *data, char *key, struct task_t *next) {
    struct task_t *tasks = (struct task_t *) malloc (sizeof(struct task_t));
    tasks->op_n = op_n;
    tasks->op = op;
    tasks->data = data;
    tasks->key = key;
    tasks->next = next;
    return tasks;
}

void queue_add_task(struct task_t *task) {
    pthread_mutex_lock(&queue_lock);
    if(queue_head == NULL) {
        queue_head = task;
    } else {
        struct task_t *tptr = queue_head;
        while(tptr->next != NULL) {
            tptr = tptr->next;
        }
        tptr->next = task;
    }
    pthread_cond_signal(&queue_not_empty);
    pthread_mutex_unlock(&queue_lock);  
}

struct task_t *queue_get_task() {
    pthread_mutex_lock(&queue_lock);
    while(queue_head == NULL) {
        pthread_cond_wait(&queue_not_empty, &queue_lock);
    }
    struct task_t *task = queue_head;
    queue_head = task->next;
    pthread_mutex_unlock(&queue_lock);
    return task;
}

void addZooIPP(char * port, char * zooIpPort) {
    zooIpAndPort = zooIpPort;
    serverPort = port;
}