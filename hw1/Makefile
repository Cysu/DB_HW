AllObjects = FuzzySearch.o FZ_indexer.o

CC = g++
CPPFLAGS = -w -O3
#LDFLAGS = -lrt

FuzzySearch : $(AllObjects)
	$(CC) -o FuzzySearch $(LDFLAGS) $(AllObjects)
	
FuzzySearch.o : main.cc FZ_indexer.o
	$(CC) -c -o $@ $(CPPFLAGS) $<

FZ_indexer.o : FZ_indexer.cc FZ_indexer.h
	$(CC) -c -o $@ $(CPPFLAGS) $<

.PHONY: clean
clean :
	rm *.o FuzzySearch
