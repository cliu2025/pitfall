all: test

test: ../src/pitfall-v3/main.c ../src/pitfall-v3/asm.S
	$(CC) $^ -o $@ -D_GNU_SOURCE

clean:
	rm -rf test
