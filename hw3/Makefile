AllObjects = AppEntExt.o AEE_indexer.o

CC = g++
CPPFLAGS = -w -O3

AppEntExt : $(AllObjects)
	$(CC) -o AppEntExt $(LDFLAGS) $(AllObjects)
	
AppEntExt.o : main.cc AEE_indexer.o
	$(CC) -c -o $@ $(CPPFLAGS) $<

AEE_indexer.o : AEE_indexer.cc AEE_indexer.h
	$(CC) -c -o $@ $(CPPFLAGS) $<

.PHONY: clean
clean :
	rm *.o AppEntExt
