CC=gcc
CFLAGS=-Wall -g
LIBS=-lsqlite3 -lcmocka

all: main test

main: src/main.c src/db.c src/queries.c
	$(CC) -o main src/main.c src/db.c src/queries.c $(CFLAGS) $(LIBS)

test: tests/test_main.c src/db.c src/queries.c
	$(CC) -o test tests/test_main.c src/db.c src/queries.c $(CFLAGS) $(LIBS)

clean:
	rm -f main test *.o