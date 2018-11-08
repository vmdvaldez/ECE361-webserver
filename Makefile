all: common.o client.o server.o
	gcc client.c ./object/common.o -o deliver
	gcc server.c ./object/common.o -o server

common.o:
	gcc common.c -c -o ./object/common.o

client.o: 
	gcc client.c -c -o ./object/client.o

server.o:
	gcc server.c -c	-o ./object/server.o


delfiles:
	rm -r ./serverfiles/*

clean:
	rm -r ./object/*
	rm server
	rm deliver