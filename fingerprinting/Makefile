all: test

test: main.c asm.S
	$(CC) $^ -o $@ -D_GNU_SOURCE

clean:
	rm -rf test
