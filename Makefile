CC = gcc
CCFLAGS = -W -Wall

BINARY_NAME = netspeed

all:
	$(CC) $(CCFLAGS) Main.c -o $(BINARY_NAME)

clean:
	rm -r $(BINARY_NAME)