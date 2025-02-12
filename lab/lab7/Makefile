CC = gcc

GAME_SRC = game.c
GAME_TARGET = game

GUESS_SRC = guess.c
GUESS_TARGET = guess

.PHONY: all clean

all: $(GAME_TARGET) $(GUESS_TARGET)

clean:
	rm -f $(GAME_TARGET) $(GUESS_TARGET)
	
$(GAME_TARGET): $(GAME_SRC)
	$(CC) -o $@ $^

$(GUESS_TARGET): $(GUESS_SRC)
	$(CC) -o $@ $^