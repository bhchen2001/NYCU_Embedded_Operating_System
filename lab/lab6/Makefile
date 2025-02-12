CC = gcc
CC_FLAGS = -Wall -Wextra -Werror -lpthread

SERVER_SRC = server.c
SERVER_TARGET = server

CLIENT_SRC = client.c
CLIENT_TARGET = client

all: $(SERVER_TARGET) $(CLIENT_TARGET)

$(SERVER_TARGET): $(SERVER_SRC)
	$(CC) -o $@ $^

$(CLIENT_TARGET): $(CLIENT_SRC)
	$(CC) -o $@ $^

clean:
	rm -f $(SERVER_TARGET) $(CLIENT_TARGET)