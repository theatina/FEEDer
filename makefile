CC=gcc
CFLAGS=-I.
DEPS = semun.h
OBJ = main.o semun.o 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

feeder: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

main: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

clean:
	rm -rf $(OBJ) feeder  statsfile.txt

