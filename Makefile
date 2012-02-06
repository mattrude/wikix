#CFLAGS = -g
#CFLAGS_LIB = -g -c
CFLAGS = -Wno-pointer-sign -g
CFLAGS_LIB = -Wno-pointer-sign -g -c
CC = gcc
LD = ld
AR = ar
 
all:  wikix 
 
libcutf8.so: utf8.o
	$(LD) -shared -lc -o libcutf8.so utf8.o /usr/lib/libc.a
 
libcutf8.a: utf8.o
	$(AR) r libcutf8.a utf8.o
 
wikix: wikix.c libcutf8.a 
	$(CC) $(CFLAGS) wikix.c -o wikix libcutf8.a -lssl 
 
clean:
	rm -f *.o wikix
 
install: all
	install -m 755 wikix /usr/bin
	install -m 644 libcutf8.so /usr/lib
	install -m 644 libcutf8.a /usr/lib
