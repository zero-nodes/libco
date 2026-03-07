CC = clang
CFLAGS = -I./include/ \
		 -D_GNU_SOURCE \
		 -O2 \
		 -std=c23

LIBRARY = build/libco.a

SRCS = src/context/context_switch.S \
       src/context/context_run.S \
       src/context/context_save.S \
       src/context/context.c \
       src/libco/coroutine.c \
       src/libco/scheduler.c \
       src/libco/async_socket.c \
       src/libco/async_time.c \

OBJS = $(SRCS:src/%.c=build/%.o)
OBJS += $(SRCS:src/%.S=build/%.o)

all: $(LIBRARY)

$(LIBRARY): $(OBJS)
	@mkdir -p $(dir $@)
	ar rcs $@ $(OBJS)

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: src/%.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build

.PHONY: all clean
