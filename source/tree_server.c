/* 
Projecto de SD - Fase 4
Grupo: 45
António Pereira - 50320
Guilherme Lopes - 52761
Ricardo Banon - 42035
*/

#include "network_server.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    signal(SIGPIPE, SIG_IGN);
    int listening_socket;
    int run = 0;	
	if (argc < 3) {
        printf("Uso: tree_server <Porta TCP> <IP ZooKeeper>:<Porta>");
        printf("Exemplo de uso: tree_server 5000 127.0.0.1:2181\n");
		return -1;
	}
    short port = atoi(argv[1]);
    printf("Starting\n");
    listening_socket = network_server_init(port);
    printf("Getting tree\n");
    addZooIPP(argv[1], argv[2]);
    tree_skel_init();
    printf("Listening\n");
    run = network_main_loop(listening_socket);
    printf("Closing\n");
    tree_skel_destroy();
    return run;
}