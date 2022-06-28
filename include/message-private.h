#ifndef _MESSAGE_PRIVATE_H
#define _MESSAGE_PRIVATE_H

/* 
Projecto de SD - Fase 4
Grupo: 45
António Pereira - 50320
Guilherme Lopes - 52761
Ricardo Banon - 42035
*/

#include "sdmessage.pb-c.h"

struct message_t {
    struct _MessageT *message;
};

int write_all(int sock, char *buf, int len);

int read_all(int sock, char *buf, int len);

#endif