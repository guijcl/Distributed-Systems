#ifndef _TREE_SKEL_H
#define _TREE_SKEL_H

/* 
Projecto de SD - Fase 4
Grupo: 45
António Pereira - 50320
Guilherme Lopes - 52761
Ricardo Banon - 42035
*/

#include "sdmessage.pb-c.h"
#include "message-private.h"
#include "tree.h"

//adicionamos o nosso message-private para tratar do message_t
/* Inicia o skeleton da árvore.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). 
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */
int tree_skel_init();

/* Liberta toda a memória e recursos alocados pela função tree_skel_init.
 */
void tree_skel_destroy();

/* Executa uma operação na árvore (indicada pelo opcode contido em msg)
 * e utiliza a mesma estrutura message_t para devolver o resultado.
 * Retorna 0 (OK) ou -1 (erro, por exemplo, árvore nao incializada)
*/
int invoke(struct message_t *msg);

/* Verifica se a operação identificada por op_nfoi executada.
*/
int verify(int op_n);

/*Função do thread secundário que vai processar pedidos de escrita.
*/
void * process_task(void * params);

void addZooIPP(char * port, char * zooipPort);

#endif
