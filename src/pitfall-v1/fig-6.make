CFLAGS = -D_GNU_SOURCE

test_byte_size ?= 0

ifeq ($(test_byte_size), 0)
  CFLAGS += 
else
  CFLAGS += -DTEST_BYTE_SIZE=$(test_byte_size)
endif

bin	:= ../../bin
obj	:= $(bin)/pitfall-v1-eval

all: $(obj)

$(obj): eval.c
	mkdir -p $(bin)
	$(CC) $^ -o $@ $(CFLAGS)

clean:
	rm -rf $(obj)