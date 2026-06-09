all: test eval

test: main.c asm.S
	$(CC) $^ -o $@ -D_GNU_SOURCE

eval: eval.c asm.S
	$(CC) $^ -o $@ -D_GNU_SOURCE

clean:
	rm -rf test eval
