all: test

test: src/main.c src/asm.S
	$(CC) $^ -o $@ -D_GNU_SOURCE

clean:
	rm -rf test
