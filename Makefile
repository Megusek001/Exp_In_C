CC = gcc
CFLAGS = -pthread

all: programa

programa: programa.c
	$(CC) $(CFLAGS) -o  programa programa.c

clean:
	rm -fm programa
