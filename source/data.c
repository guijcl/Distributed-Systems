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
#include "data.h"


/* Função que cria um novo elemento de dados data_t e reserva a memória
 * necessária, especificada pelo parâmetro size 
 */
struct data_t *data_create(int size) {
    if (size < 1)
        return NULL;

    struct data_t *nData;
    nData = (struct data_t *) malloc (sizeof (struct data_t));
    
    if (nData == NULL)
        return NULL;
    
    nData->datasize = size;
    nData->data = malloc(size);

    if (nData->data == NULL) {
        free(nData);
        return NULL;
    }

    return nData;
}

/* Função idêntica à anterior, mas que inicializa os dados de acordo com
 * o parâmetro data.
 */
struct data_t *data_create2(int size, void *data) {
    if(size < 1 || data == NULL)
        return NULL;
    
    struct data_t *nData;
    nData = (struct data_t *) malloc (sizeof (struct data_t));

    if (nData == NULL)
        return NULL;

    nData->datasize = size;
    nData->data = data;

    if (nData->data == NULL) {
        free(nData);
        return NULL;
    }

    return nData;
}

/* Função que elimina um bloco de dados, apontado pelo parâmetro data,
 * libertando toda a memória por ele ocupada.
 */
void data_destroy(struct data_t *data) {
    if(data != NULL) {
        free(data->data);
        free(data);
    }
}

/* Função que duplica uma estrutura data_t, reservando a memória
 * necessária para a nova estrutura.r
 */
struct data_t *data_dup(struct data_t *data) {
    if(data == NULL || data->data == NULL)
        return NULL;

    if (data->datasize < 1)
        return NULL;

    struct data_t *nData;
    nData = data_create(data->datasize);

    memcpy(nData->data, data->data, data->datasize);
    
    return nData;
}

/* Função que substitui o conteúdo de um elemento de dados data_t.
*  Deve assegurar que destroi o conteúdo antigo do mesmo.
*/
void data_replace(struct data_t *data, int new_size, void *new_data) {
    if(data != NULL) {
        free(data->data);
        data->datasize = new_size;
        data->data = new_data;
    }
}