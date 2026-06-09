bin	:= ../../bin
obj	:= $(bin)/microbenchmark-fig-2

all: $(obj)

$(obj): fig-2.c fig-2.S
	mkdir -p $(bin)
	$(CC) $^ -o $@

clean:
	rm -rf $(obj)