# Makefile for Assignment 3, generates multiple binaries


CC = gcc
CFLAGS = -g -pthread
F_OUTPUT = text_searching_server
S_OUTPUT = client2
T_OUTPUT = client1

build: text_searching_server client2 client1

$(F_OUTPUT):  WordSearch.o Helper_string.o server.o
	$(CC) $(CFLAGS) WordSearch.o  Helper_string.o server.o -o text_searching_server

$(S_OUTPUT):  WordSearch.o Helper_string.o client.o
	$(CC) $(CFLAGS) WordSearch.o  Helper_string.o client.o -o client2

$(T_OUTPUT):  WordSearch.o Helper_string.o client1.o
	$(CC) $(CFLAGS) WordSearch.o  Helper_string.o client1.o -o client1
	
WordSearch.o: WordSearch.c WordSearch.h
	$(CC) $(CFLAGS) -c WordSearch.c -o WordSearch.o

Helper_string.o: Helper_string.c helper_string.h
	$(CC) $(CFLAGS) -c Helper_string.c -o Helper_string.o

server.o: text_searching_server.c
	$(CC) $(CFLAGS) -c text_searching_server.c -o server.o

client.o: client2.c
	$(CC) $(CFLAGS) -c client2.c -o client.o

client1.o: client1.c
	$(CC) $(CFLAGS) -c client1.c -o client1.o

clean:
	rm WordSearch.o Helper_string.o  server.o client.o client2 text_searching_server client1.o client1