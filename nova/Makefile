CFLAGS = -O2 -W -Wall

OBJ = dumprb.o rb.o radix50.o

dumprb : $(OBJ)
#	$(CC) -o $@ $> $(CFLAGS)  ## uncomment this line if using BSD make

clean : 
	rm -f dumprb divtest $(OBJ)

rb.o : rb.h
dumprb.o : rb.h
radix50.o : rb.h

divtest: divtest.c
	$(CC) $(CFLAGS) -o $@ $^
