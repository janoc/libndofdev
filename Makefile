CFLAGS=-pipe -Wall -Wunused-result
LDFLAGS=-lSDL

all:		libndofdev.a ndofdev_test

libndofdev.a:	ndofdev.o
		$(AR) -r $@ $<

ndofdev_test:	ndofdev_test.o libndofdev.a
		$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
		-rm -f *.a *.o ndofdev_test
