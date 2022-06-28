/* 
Projecto de SD - Fase 4
Grupo: 45
António Pereira - 50320
Guilherme Lopes - 52761
Ricardo Banon - 42035
*/

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "entry.h"

/* Função que cria uma entry, reservando a memória necessária e
 * inicializando-a com a string e o bloco de dados passados.
 */
struct entry_t *entry_create(char *key, struct data_t *data) {
    struct entry_t *nEntry;
    nEntry = (struct entry_t *) malloc (sizeof (struct entry_t));
    
    if (nEntry == NULL) 
        return NULL;

    nEntry->key = key;

    nEntry->value = data;

    return nEntry;
}

/* Função que inicializa os elementos de uma entry com o
 * valor NULL.
 */
void entry_initialize(struct entry_t *entry) {
    entry = entry_create(NULL, NULL);
}


/* Função que elimina uma entry, libertando a memória por ela ocupada
 */
void entry_destroy(struct entry_t *entry) {
    if(entry != NULL) {
        data_destroy(entry->value);
        if(entry->key != NULL)
            free(entry->key);
        free(entry);
    }
}


/* Função que duplica uma entry, reservando a memória necessária para a
 * nova estrutura.
 */
struct entry_t *entry_dup(struct entry_t *entry) {
    if(entry == NULL) 
        return NULL;
    if(entry->key == NULL || entry->value == NULL)
        return NULL;
    
    struct entry_t *entryRes;
    entryRes = entry_create(strdup(entry->key), data_dup(entry->value));

    return entryRes;
}

/* Função que substitui o conteúdo de uma entrada entry_t.
*  Deve assegurar que destroi o conteúdo antigo da mesma.
*/
void entry_replace(struct entry_t *entry, char *new_key, struct data_t *new_value) {
    free(entry->key);
    entry->key = new_key; 
    data_destroy(entry->value);
    entry->value = new_value;
}

/* Função que compara duas entradas e retorna a ordem das mesmas.
*  Ordem das entradas é definida pela ordem das suas chaves.
*  A função devolve 0 se forem iguais, -1 se entry1<entry2, e 1 caso contrário.
*/
int entry_compare(struct entry_t *entry1, struct entry_t *entry2) {
    int cmpres = strcmp(entry1->key, entry2->key);

    if(cmpres < 0) 
        return -1;
    else if(cmpres > 0) 
        return 1;
    else 
        return 0;
}