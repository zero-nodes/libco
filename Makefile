CC = gcc
CFLAGS = -I./src/ -g -O2

TARGET = build/app

SRCS = src/main.c \
       src/context/context_switch.S \
       src/context/context_run.S \
	   src/context/context_save.S \
       src/context/context.c \
	   src/colib/coroutine.c \
	   src/colib/scheduler.c

OBJS = $(SRCS:%.c=build/%.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(OBJS) -o $(TARGET)

build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: %.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build

.PHONY: all clean
