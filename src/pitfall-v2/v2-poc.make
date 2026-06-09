CFLAGS = -D_GNU_SOURCE

bin	:= ../../bin
obj	:= $(bin)/pitfall-v2-poc

all: $(obj)

$(obj): poc.c
	mkdir -p $(bin)
	$(CC) $^ -o $@ $(CFLAGS)

clean:
	rm -rf $(obj)