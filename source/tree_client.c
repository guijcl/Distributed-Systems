/* 
Projecto de SD - Fase 4
Grupo: 45
António Pereira - 50320
Guilherme Lopes - 52761
Ricardo Banon - 42035
*/

#include "client_stub_private.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

/*
	Programa cliente para manipular uma arvore remota.
	Os comandos introduzido no programa não deverão exceder
	80 carateres.

	Uso: tree-client <ip servidor zoo>:<porta servidor zoo>
	Exemplo de uso: ./tree_client 10.101.148.144:54321
*/
int main(int argc, char **argv) {
    signal(SIGPIPE, SIG_IGN);
    char input[81];
    struct rtree_t *rtree;
    if (argc < 2) {
        printf("Insert <Port> for connection\n");
        printf("Closing\n");
        return -1;
    }
    rtree = rtree_connect(argv[1]);
    if(rtree == NULL) {
        printf("Connection Closed Error\n");
        printf("Closing\n");
        return -1;
    }
    char *comando = NULL;
    char *key = NULL;
    char *dataf = NULL;
    printf("Client open\n");
    printf("Ready\n");
    printf(">>> ");
    if(fgets(input, 80, stdin) == NULL) {
        printf("Write a command\n");
    }
    while((int) strcspn(input, "\n") <= 1) {
        printf("Write a command\n");
        printf(">>> ");
        if(fgets(input, 80, stdin) == NULL) {
            printf("Write a command\n");
            printf(">>> ");
        }
    }
    int s = (int) strcspn(input, "\n");
    input[s] = '\0'; 
    comando = strtok(input, " ");
    int res = 0;
    while(strcmp(comando, "quit") != 0) {
        printf("Checking\n");
        if(strcmp(comando, "put") == 0) {
            key = strtok(NULL, " ");
            dataf = strtok(NULL, " ");
            if(key == NULL || dataf == NULL) {
                printf("Invalid Arguments\n");
                printf("Avoid Uppercase\n");
                printf("Currently available operations:\n");
                printf("put <key> <data>\n");
                printf("get <key>\n");
                printf("del <key>\n");
                printf("size\n");
                printf("height\n");
                printf("getkeys\n");
                printf("verify <op_n>\n"); 
                res = -1;      
            } else {
            
                struct data_t *dataput;
                dataput = data_create2(sizeof(dataf), dataf);
                struct entry_t *entryput;
                entryput = entry_create(key, dataput);
                printf("Calling put\n");
                res = rtree_put(rtree, entryput);
                printf("Opcode is: %d \n", res);
            }
            
        } else if(strcmp(comando, "get") == 0) {
            key = strtok(NULL, " ");
            if(key == NULL) {
                printf("Invalid Arguments\n");
                printf("Avoid Uppercase\n");
                printf("Currently available operations:\n");
                printf("put <key> <data>\n");
                printf("get <key>\n");
                printf("del <key>\n");
                printf("size\n");
                printf("height\n");
                printf("getkeys\n");
                printf("verify <op_n>\n");
                res = -1;       
            } else {
                printf("Calling get\n");
                if(rtree_get(rtree, key) == NULL) {
                    res = -1;
                    printf("Get Failed Not Found\n");
                }else {
                    res = 0;
                    printf("Get Done\n");
                }
            }

        } else if(strcmp(comando, "del") == 0) {
            key = strtok(NULL, " ");
            if(key == NULL) {
                printf("Invalid Arguments\n");
                printf("Avoid Uppercase\n");
                printf("Currently available operations:\n");
                printf("put <key> <data>\n");
                printf("get <key>\n");
                printf("del <key>\n");
                printf("size\n");
                printf("height\n");
                printf("getkeys\n");
                printf("verify <op_n>\n");
                res = -1;       
            } else {
                printf("Calling del\n");
                res = rtree_del(rtree, key);
                printf("Opcode is: %d \n", res);
            }

        } else if(strcmp(comando, "size") == 0) {
            printf("Calling size\n");
            res = rtree_size(rtree);
            printf("tree size: %d", res);
        } else if(strcmp(comando, "height") == 0) {
            printf("Calling height\n");
            res = rtree_height(rtree);
            printf("tree height: %d", res);
        } else if(strcmp(comando, "getkeys") == 0) {
            printf("Calling getkeys\n");
            char **keys = rtree_get_keys(rtree);
        } else if(strcmp(comando, "verify") == 0) {
            key = strtok(NULL, " ");
            if(key == NULL) {
                printf("Invalid Arguments\n");
                printf("Avoid Uppercase\n");
                printf("Currently available operations:\n");
                printf("put <key> <data>\n");
                printf("get <key>\n");
                printf("del <key>\n");
                printf("size\n");
                printf("height\n");
                printf("getkeys\n");
                printf("verify <op_n>\n");
                res = -1;       
            } else {
                printf("Verifying\n");
                res = rtree_verify(rtree, atoi(key));
                if(res > 0) {
                    printf("Op %d has been executed\n", res);
                } else {
                    printf("Op %d hasn't been executed\n", res);
                }
            }
        } else {
            printf("Invalid Arguments\n");
            printf("Avoid Uppercase\n");
            printf("Currently available operations:\n");
            printf("put <key> <data>\n");
            printf("get <key>\n");
            printf("del <key>\n");
            printf("size\n");
            printf("height\n");
            printf("getkeys\n");
            printf("verify <op_n>\n");
        }
        printf("Done\n");
        printf(">>> ");
        if(fgets(input, 80, stdin) == NULL) {
            printf("Write a command\n");
        }

        while((int) strcspn(input, "\n") <= 1) {
            printf("Write a command\n");
            printf(">>> ");
            if(fgets(input, 80, stdin) == NULL) {
                printf("Write a command\n");
                printf(">>> ");
            }
        }
        int s = (int) strcspn(input, "\n");
        input[s] = '\0'; 
        comando = strtok(input, " ");
    }
    printf("Closing\n");
    
    rtree_disconnect(rtree);
    return res;
}