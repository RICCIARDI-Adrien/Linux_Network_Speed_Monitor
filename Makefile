CC = gcc
CCFLAGS = -W -Wall

BINARY_NAME = netspeed
INSTALLATION_PATH = /usr/bin

all:
	$(CC) $(CCFLAGS) Main.c -o $(BINARY_NAME)

clean:
	rm -r $(BINARY_NAME)

install:
	install $(BINARY_NAME) $(INSTALLATION_PATH)

uninstall:
	rm -f $(INSTALLATION_PATH)/$(BINARY_NAME)
