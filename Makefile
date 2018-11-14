CC = g++
eflag = -pthread -std=c++11 -o 
FLAGS = -std=c++11 -c -o 
o_dir = ./object/

all: common.o client.o server.o
	$(CC)  client.cpp $(o_dir)common.o $(eflag) deliver
	$(CC)  server.cpp $(o_dir)common.o $(eflag) server

common.o:
	$(CC)  common.cpp $(FLAGS) $(o_dir)common.o

client.o: 
	$(CC)  client.cpp $(FLAGS) $(o_dir)client.o

server.o:
	$(CC)  server.cpp $(FLAGS) $(o_dir)server.o


delfiles:
	rm -r ./serverfiles/*

clean:
	rm -r $(o_dir)*
	rm server
	rm deliver
