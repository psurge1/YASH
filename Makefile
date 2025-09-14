CC = gcc -std=c11
CFLAGS = -I. -g

LIBS = -lreadline

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
TARGET = yash

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LIBS)

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)