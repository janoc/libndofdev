CFLAGS=-pipe -Wall -Wunused-result -fPIC
ifeq ($(USE_SDL3),1)
CFLAGS := $(CFLAGS) -DUSE_SDL3
LDFLAGS=-lSDL3
else ifeq ($(USE_SDL2),1)
CFLAGS := $(CFLAGS) -DUSE_SDL2
LDFLAGS=-lSDL2
else
LDFLAGS=-lSDL
endif


all:		libndofdev.so libndofdev.a ndofdev_test

libndofdev.so:	ndofdev.o
		$(CC) -shared -o $@ $< $(LDFLAGS)

libndofdev.a:	ndofdev.o
		$(AR) -l "$(LDFLAGS)" -r $@ $<

ndofdev_test:	ndofdev_test.o libndofdev.a
		$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
		-rm -f *.so *.a *.o ndofdev_test
