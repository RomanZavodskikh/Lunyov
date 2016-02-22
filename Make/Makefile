CFLAGS=-MD
.PHONY: all clean

all: a.out
-include *.d

a.out: 1.o 2.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm *.o *.d a.out
