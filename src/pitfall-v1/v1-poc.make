CFLAGS = -D_GNU_SOURCE

bin	:= ../../bin
obj	:= $(bin)/pitfall-v1-poc
secret ?= ''

ifneq ($(strip $(secret)),)
CFLAGS += -DSECRET_STRING='"$(secret)"'
endif

all: $(obj)

$(obj): poc.c
	mkdir -p $(bin)
	$(CC) $^ -o $@ $(CFLAGS)

clean:
	rm -rf $(obj)