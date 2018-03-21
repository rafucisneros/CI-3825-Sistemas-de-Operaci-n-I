CC = gcc
CFLAGS = -pthread -I.
DEPS = hash.h
OBJS = main.o hash.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

indice: $(OBJS)
	$(CC) $? -o $@ $(CFLAGS)

clean:
	-rm -f indice
	-rm -f $(OBJS)