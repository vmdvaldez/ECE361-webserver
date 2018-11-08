CC = gcc
FLAGS = -c -o
o_dir = ./object/

all: common.o client.o server.o
	$(CC)  client.c $(o_dir)common.o -o deliver
	$(CC)  server.c $(o_dir)common.o -o server

common.o:
	$(CC)  common.c $(FLAGS) $(o_dir)common.o

client.o: 
	$(CC)  client.c $(FLAGS) $(o_dir)client.o

server.o:
	$(CC)  server.c $(FLAGS) $(o_dir)server.o


delfiles:
	rm -r ./serverfiles/*

clean:
	rm -r $(o_dir)*
	rm server
	rm deliver