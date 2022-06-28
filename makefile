# Projecto de SD - Fase 4
# Grupo: 45
# António Pereira - 50320
# Guilherme Lopes - 52761
# Ricardo Banon - 42035

PROTOC_DIR=/usr/local/
SHELL:=/bin/bash 
BIN = binary/
OBJ= object/
OBJECTOS = sdmessage.pb-c.o data.o entry.o tree.o message.o client_stub.o network_client.o client-lib.o tree_client.o tree_skel.o network_server.o tree_server.o server-lib.o
OBJCLEAN = $(OBJ)sdmessage.pb-c.o $(OBJ)sdmessage.pb-c.d $(OBJ)data.o $(OBJ)entry.o $(OBJ)tree.o $(OBJ)message.o $(OBJ)tree_skel.o $(OBJ)network_server.o $(OBJ)tree_server.o $(LIB)server-lib.o $(OBJ)client_stub.o $(OBJ)network_client.o $(LIB)client-lib.o $(OBJ)tree_client.o
EXES = tree-server tree-client
EXESCLEAN = $(BIN)tree-server $(BIN)tree-client
LIB = lib/
INCLUDE = include/
LDFLAGS = ${PROTOC_DIR}lib/libprotobuf-c.a
SRC = source/
CC = gcc
CPROTOCFLAGS = -Wall -g -ggdb -O2 -I ${PROTOC_DIR}include/ -c -v#gdb debugger -O0 -v -da -Q -I 
CFLAGS = -Wall -g -pthread -O2 -I include/ -c -v#gdb debugger -O0 -v -da -Q -I
run : sdmessage.pb-c.c $(OBJECTOS) $(EXES)

data.o: $(INCLUDE)data.h
	$(CC) $(CFLAGS) $(SRC)data.c -o $(OBJ)data.o

tree.o: $(INCLUDE)data.h $(INCLUDE)tree-private.h $(INCLUDE)entry.h
	$(CC) $(CFLAGS) $(SRC)tree.c -o $(OBJ)tree.o

entry.o: $(INCLUDE)entry.h $(INCLUDE)data.h
	$(CC) $(CFLAGS) $(SRC)entry.c -o $(OBJ)entry.o

sdmessage.pb-c.c: sdmessage.proto
	protoc-c sdmessage.proto --c_out=.
	mv sdmessage.pb-c.h include/
	mv sdmessage.pb-c.c source/

sdmessage.pb-c.o: $(SRC)sdmessage.pb-c.c
	$(CC) $(CFLAGS) -MD $(SRC)sdmessage.pb-c.c -o $(OBJ)sdmessage.pb-c.o

message.o: $(INCLUDE)message-private.h
	$(CC) $(CFLAGS) $(SRC)message.c -o $(OBJ)message.o

tree_skel.o: $(INCLUDE)task.h $(INCLUDE)tree_skel.h $(INCLUDE)message-private.h $(INCLUDE)network_client.h $(INCLUDE)client_stub_private.h
	$(CC) -pthread -DTHREADED $(CFLAGS) $(SRC)tree_skel.c -o $(OBJ)tree_skel.o

network_server.o: $(INCLUDE)network_server.h
	$(CC) $(CFLAGS) $(SRC)network_server.c -o $(OBJ)network_server.o

tree_server.o: $(INCLUDE)network_server.h
	$(CC) $(CFLAGS) -DTHREADED $(SRC)tree_server.c -o $(OBJ)tree_server.o

server-lib.o:
	ld -r $(OBJ)data.o $(OBJ)entry.o $(OBJ)tree.o $(OBJ)sdmessage.pb-c.o $(OBJ)message.o $(OBJ)tree_skel.o $(OBJ)network_server.o $(OBJ)client_stub.o $(OBJ)network_client.o -o $(LIB)server-lib.o

tree-server: $(LIB)server-lib.o
	$(CC) -pthread -DTHREADED $(LIB)server-lib.o $(OBJ)tree_server.o $(LDFLAGS) -o $(BIN)tree-server -lzookeeper_mt

client_stub.o: $(INCLUDE)client_stub_private.h $(INCLUDE)message-private.h $(INCLUDE)network_client.h
	$(CC) $(CFLAGS) -DTHREADED $(SRC)client_stub.c -o $(OBJ)client_stub.o

network_client.o: $(INCLUDE)network_client.h $(INCLUDE)client_stub_private.h $(INCLUDE)message-private.h
	$(CC) $(CFLAGS) $(SRC)network_client.c -o $(OBJ)network_client.o

client-lib.o:
	ld -r $(OBJ)data.o $(OBJ)entry.o $(OBJ)sdmessage.pb-c.o $(OBJ)message.o $(OBJ)client_stub.o $(OBJ)network_client.o -o $(LIB)client-lib.o

tree_client.o: $(INCLUDE)client_stub.h
	$(CC) $(CFLAGS) -DTHREADED $(SRC)tree_client.c -o $(OBJ)tree_client.o

tree-client: $(LIB)client-lib.o
	$(CC) $(LIB)client-lib.o $(OBJ)tree_client.o $(LDFLAGS) -o $(BIN)tree-client -lzookeeper_mt

clean:
	rm $(EXESCLEAN)
	rm $(OBJCLEAN)
