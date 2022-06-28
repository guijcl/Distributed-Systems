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
#include "data.h"
#include "entry.h" 
#include "tree-private.h"

/* Função para criar uma nova árvore tree vazia.
 * Em caso de erro retorna NULL.
 */
struct tree_t *tree_create() {
    struct tree_t *tree;
    tree = (struct tree_t *) malloc (sizeof (struct tree_t));
    
    if(tree == NULL)
        return NULL;

    tree->entry = NULL;
    tree->left = NULL;
    tree->right = NULL;

    return tree;
}

/* Função para libertar toda a memória ocupada por uma árvore.
 */
void tree_destroy(struct tree_t *tree) {
    if(tree != NULL) {
        if(tree->left != NULL)
            tree_destroy(tree->left);
        if(tree->right != NULL)
            tree_destroy(tree->right);
        entry_destroy(tree->entry);
        free(tree);
    }
}

/* Função para adicionar um par chave-valor à árvore.
 * Os dados de entrada desta função deverão ser copiados, ou seja, a
 * função vai *COPIAR* a key (string) e os dados para um novo espaço de
 * memória que tem de ser reservado. Se a key já existir na árvore,
 * a função tem de substituir a entrada existente pela nova, fazendo
 * a necessária gestão da memória para armazenar os novos dados.
 * Retorna 0 (ok) ou -1 em caso de erro.
 */
int tree_put(struct tree_t *tree, char *key, struct data_t *value) {
    if(tree == NULL || key == NULL || value == NULL)
        return -1;

    char *tempKey = strdup(key);

    struct data_t *tempVal;
    tempVal = data_dup(value);

    struct tree_t *root;
    root = tree;
    
    struct entry_t *tempEntry;
    tempEntry = entry_create(tempKey, tempVal);
    tree_put_aux(tree, tempEntry);
    
    if(tree == NULL)
        return -1;

    tree = root;

    return 0;
}

struct tree_t *tree_put_aux(struct tree_t *tree, struct entry_t *tempEntry) {
    if(tree == NULL) {
        tree = tree_create();
        tree->entry = entry_dup(tempEntry);
        return tree;
    }

    if(tree->entry == NULL) {
        tree->entry = entry_dup(tempEntry);
        return tree;
    }

    int result = entry_compare(tree->entry, tempEntry);
    if(result == 0) {
        entry_replace(tree->entry, tempEntry->key, tempEntry->value);
        return tree;
    } else if (result > 0) {
        tree->left = tree_put_aux(tree->left, tempEntry);
    } else if (result < 0) {
        tree->right = tree_put_aux(tree->right, tempEntry);
    }

    return tree;
}

/* Função para obter da árvore o valor associado à chave key.
 * A função deve devolver uma cópia dos dados que terão de ser
 * libertados no contexto da função que chamou tree_get, ou seja, a
 * função aloca memória para armazenar uma *CÓPIA* dos dados da árvore,
 * retorna o endereço desta memória com a cópia dos dados, assumindo-se
 * que esta memória será depois libertada pelo programa que chamou
 * a função.
 * Devolve NULL em caso de erro.
 */
struct data_t *tree_get(struct tree_t *tree, char *key) {
    if(tree == NULL)
        return NULL;

    if(key == NULL || tree->entry== NULL)
        return NULL;

    if (strcmp(tree->entry->key, key) == 0)
        return data_dup(tree->entry->value);
    else if (strcmp(tree->entry->key, key) < 0)
        return tree_get(tree->right, key);
    else if (strcmp(tree->entry->key, key) > 0)
        return tree_get(tree->left, key);

    return NULL;
}

/* Função para remover um elemento da árvore, indicado pela chave key,
 * libertando toda a memória alocada na respetiva operação tree_put.
 * Retorna 0 (ok) ou -1 (key not found).
 */
int tree_del(struct tree_t *tree, char *key) {
    if(tree == NULL || key == NULL || tree->entry == NULL)
        return -1;
    if(tree_get(tree, key) == NULL)
        return -1;

    struct tree_t *root;
    root = tree;
    if(tree_size(tree) == 1) {
        entry_destroy(tree->entry);
        tree->entry = NULL;
        return 0;
    }

    tree_del_aux(tree,NULL, key);
    if(tree == NULL)
        return -1;
    tree = root;

    return 0;
}

struct tree_t *tree_del_aux(struct tree_t *tree, struct tree_t *parent, char *key) {
    if(tree == NULL)
        return tree;

    if(key == NULL || tree->entry== NULL)
        return tree;

    int res = strcmp(tree->entry->key, key);
    if(res < 0) {
        tree->right = tree_del_aux(tree->right, tree, key);
    } else if (res > 0) {
        tree->left = tree_del_aux(tree->left, tree, key);
    } else {
        if(tree->left == NULL) {
            tree = tree->right;
            return tree;
        } else if(tree->right == NULL) {
            tree = tree->left;
            return tree;
        } else if (tree->left == NULL && tree->right == NULL) {  
            free(tree);
            tree = parent;
            return tree;
        }
        struct tree_t *temp;
        temp = inOrderS(tree->right);
        tree->right = tree_del_aux(tree->right, tree, temp->entry->key);
        tree->entry = entry_dup(temp->entry);
    }

    return tree;
}

struct tree_t *inOrderS(struct tree_t *tree) {
    struct tree_t *current;
    current = (struct tree_t *) malloc (sizeof (struct tree_t));

    if(current == NULL) {
        free(current);
        return NULL;
    }

    current = tree;
    while(current && current->left != NULL) {       
        current = current->left;
    }

    return current;
}


/* Função que devolve o número de elementos contidos na árvore.
 */
int tree_size(struct tree_t *tree) {
    if(tree == NULL)
        return 0;
    else if (tree->entry == NULL)
        return 0;

    return (tree_size(tree->left) + 1 + tree_size(tree->right));
}

/* Função que devolve a altura da árvore.
 */
int tree_height(struct tree_t *tree) {
    if(tree == NULL)
        return 0;

    int lHeight = tree_height(tree->left);
    int rHeight = tree_height(tree->right);

    if(lHeight >= rHeight)
        return (lHeight + 1);
    else
        return (rHeight + 1);
}

/* Função que devolve um array de char* com a cópia de todas as keys da
 * árvore, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária.
 */
char **tree_get_keys(struct tree_t *tree) {
    int size = tree_size(tree) + 1;
    char **array;
    array = (char **) malloc (size*(sizeof (char*)));
    
    char **arrayStart;
    arrayStart = array;
    int j = 0;

    tree_get_keys_aux(tree, array, j);
    array = arrayStart;

    return array;
}

char **tree_get_keys_aux(struct tree_t *tree, char **array, int j) {
    if(tree == NULL)
        return array;

    if(tree->entry== NULL)
        return array; 

    tree_get_keys_aux(tree->left, array, j + 1);
    array[j] = strdup(tree->entry->key);
    tree_get_keys_aux(tree->right, array, j + 1);

    return array;
}

/* Função que liberta toda a memória alocada por tree_get_keys().
 */
void tree_free_keys(char **keys) {
    free(keys);
}