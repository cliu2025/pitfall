CFLAGS = -D_GNU_SOURCE

bin	:= ../../bin
obj	:= $(bin)/pitfall-v1-poc

all: $(obj)

all: $(obj): poc.c
	mkdir -p $(bin)
	$(CC) $^ -o $@ $(CFLAGS)

clean:
	rm -rf $(obj)