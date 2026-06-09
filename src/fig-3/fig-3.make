align_size_st ?= 1
align_size_ld ?= 1

bin	:= ../../bin
obj	:= $(bin)/microbenchmark-fig-3

all: $(obj)

$(obj): fig-3.c fig-3.S
	mkdir -p $(bin)
	$(CC) $^ -o $@ -DALIGN_SIZE_ST=$(align_size_st) -DALIGN_SIZE_LD=$(align_size_ld)

clean:
	rm -rf $(obj)