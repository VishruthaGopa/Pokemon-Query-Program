# MAKEFILE

# Specify the targets and their dependencies
all: server client

server: server.o
	gcc -o server server.o -lm -lpthread

client: client.o
	gcc -o client client.o -lpthread

server.o: server.c server.h
	gcc -c server.c

client.o: client.c client.h
	gcc -c client.c

# Define the clean-> remove object files and executables
clean:
	rm -f *.o server client
