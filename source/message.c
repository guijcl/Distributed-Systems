/* 
Projecto de SD - Fase 4
Grupo: 45
António Pereira - 50320
Guilherme Lopes - 52761
Ricardo Banon - 42035
*/

#include "message-private.h"
#include <errno.h>
#include <stdio.h>
#include "inet.h"
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

int write_all(int sock, char *buf, int len) {
	int bufsize = len;
	while (len > 0) {
		int res = write(sock, buf, len);
		if (res < 0) {
			if (errno == EINTR) continue;
			return res;
		}
		buf += res;
		len -= res;
	}
	return bufsize;
}

int read_all(int sock, char *buf, int len) {
	int bufsize = len;
	while (len > 0) {
		int res = read(sock, buf, len);
		if (res < 0) {
			if (errno == EINTR) continue;
				perror("read failed: ");
			return res;
		}
		buf += res;
		len -= res;
	}
	return bufsize;
}