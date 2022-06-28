#ifndef _TASK_H
#define _TASK_H

/* 
Projecto de SD - Fase 4
Grupo: 45
António Pereira - 50320
Guilherme Lopes - 52761
Ricardo Banon - 42035
*/

struct task_t{
    int op_n; //o número da operação
    int op; //a operação a executar. op=0 se for um delete, op=1 se for um put
    char* key; //a chave a remover ou adicionar
    char* data; // os dados a adicionar em caso de put, ou NULL em caso de delete
    //adicionar campo(s) necessário(s)para implementar fila do tipo produtor/consumidor
    struct task_t *next;
};

struct task_t *task_create(int op, int op_n, char *data, char *key, struct task_t *next);

void queue_add_task(struct task_t *task);

struct task_t *queue_get_task();
#endif
