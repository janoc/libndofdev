CFLAGS=-pipe -Wall -Wunused-result 

libndofdev.a:	ndofdev.o
		$(AR) -r $@ $< 

clean:
		-rm -f *.a *.o

