#myShell
CC = gcc
SRC = myShell.c
.PHONY : clean

all:
	$(CC)  $(SRC) -o myShell -lreadline -lncurses
clean:
	-rm myShell myShell.o
	
