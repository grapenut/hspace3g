# Makefile for HSpace
CFLAGS=$(CCFLAGS) -Wno-unused-parameter -DHSPACE -I../.. -I../../hdrs -g

C_FILES := $(shell ls *.c)
O_FILES = $(C_FILES:.c=.o)

.SUFFIX:
.SUFFIX: .c .h

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

all: libhspace.a
	(cd ../..; make)

libhspace.a: hsexterns.h $(O_FILES)
	rm -f libhspace.a
	$(AR) rcs libhspace.a $(O_FILES)

clean:
	rm -f libhspace.a hsexterns.h *.o *~

backup:
	mkdir space
	cp *.txt *.c *.h Makefile mkexterns space/
	tar czvf space.tar.gz space/
	rm -rf space/
	cp space.tar.gz ~/

hsexterns.h:
	./mkexterns > hsexterns.h


extern: hsexterns.h
	

