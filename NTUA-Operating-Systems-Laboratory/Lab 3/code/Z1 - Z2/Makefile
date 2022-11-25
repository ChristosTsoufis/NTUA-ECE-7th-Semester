all :		client server

client : 	client.o aes.o message_enc.o
		gcc -o client client.o aes.o message_enc.o -lncurses

server :	server.o aes.o message_enc.o
		gcc -o server server.o aes.o message_enc.o

server.o :	server.c message_enc.h
		gcc -c -DUSE_ENC=$(USE_ENC) server.c

client.o : 	client.c message_enc.h
		gcc -c -DUSE_ENC=$(USE_ENC) client.c

aes.o :  	aes.c aes.h
		gcc -c aes.c

message_enc.o : message_enc.c
		gcc -c message_enc.c

clean :
		rm *.o client server
