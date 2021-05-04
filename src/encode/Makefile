
CC=gcc
CFLAGS=-Wall
DEPS = stegano.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

default: encoder decoder demo

encoder: encoder.o
	$(CC) -o encoder encoder.o

decoder: decoder.o
	$(CC) -o decoder decoder.o

demo: encodingdemo.o
	$(CC) -o demo encodingdemo.o

clean: 
	rm -f *.o
	rm -f encoder
	rm -f decoder
	rm -f demo
	rm -f result.png