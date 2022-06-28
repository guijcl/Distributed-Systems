#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

#include "client_stub.h"

/* 
Projecto de SD - Fase 4
Grupo: 45
António Pereira - 50320
Guilherme Lopes - 52761
Ricardo Banon - 42035
*/

struct rtree_t {
    struct entry_t *entry;
    struct rtree_t *left;
    struct rtree_t *right;
};

struct data_t *rtree_get_data(struct rtree_t *rtree, char *key);

int rtree_put_data(struct rtree_t *rtree, char *key, struct data_t *value);

/*
Func auxiliar
*/
//struct rtree_t *inOrderS(struct rtree_t *tree);

struct rtree_t *rtree_put_aux(struct rtree_t *tree, struct entry_t *tempEntry);

//struct rtree_t *rtree_del_aux(struct rtree_t *tree, struct rtree_t *parent, char *key);

//char **rtree_get_keys_aux(struct rtree_t *tree, char **array, int j);

int rtree_destroy(struct rtree_t *rtree);

int rtree_getsize(struct rtree_t *rtree);

#endif
