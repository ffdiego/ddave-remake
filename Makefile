CC = gcc
SDL2 = `sdl2-config --cflags --libs`
INCS = -std=gnu89 -Wall

SRC = lmdave.c
OBJ = lmdave

all: $(SRC)
	$(CC) $(SRC) $(INCS) $(SDL2) -o $(OBJ)