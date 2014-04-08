CFLAGS=-pipe

libndofdev.a:	ndofdev.o
		$(AR) -r $@ $< 

clean:
		-rm -f *.a *.o

