CC = g++
CPPFLAGS = -w -O3
#LDFLAGS = -lrt

simjoin: main.cc simJoin.cc simJoin.h
	$(CC) $(CPPFLAGS) -o simjoin $(LDFLAGS) *.cc

.PHONY: clean
clean :
	rm simjoin
