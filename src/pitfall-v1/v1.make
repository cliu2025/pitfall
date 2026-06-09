CFLAGS = -D_GNU_SOURCE

bin	:= ../../bin
obj	:= $(bin)/pitfall-v1-eval

all: $(obj)

$(obj): eval.c
	mkdir -p $(bin)
	$(CC) $^ -o $@ $(CFLAGS)

clean:
	rm -rf $(obj)