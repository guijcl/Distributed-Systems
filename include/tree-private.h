#ifndef _TREE_PRIVATE_H
#define _TREE_PRIVATE_H

#include "tree.h"
#include "data.h"
#include "entry.h"

/* 
Projecto de SD - Fase 4
Grupo: 45
António Pereira - 50320
Guilherme Lopes - 52761
Ricardo Banon - 42035
*/

struct tree_t {
	struct entry_t *entry;
	struct tree_t *left;
	struct tree_t *right;
};

/*
Func auxiliar
*/
struct tree_t *inOrderS(struct tree_t *tree);

struct tree_t *tree_put_aux(struct tree_t *tree, struct entry_t *tempEntry);

struct tree_t *tree_del_aux(struct tree_t *tree, struct tree_t *parent, char *key);

char **tree_get_keys_aux(struct tree_t *tree, char **array, int j);

#endif
