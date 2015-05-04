CC = g++
CFLAGS = -g -std=c++11 -pedantic -Wall -Werror
LDLIBS = -lnuma

srcDir := ../

HybridMemory.o: $(srcDir)/HybridMemory.cpp $(srcDir)/HybridMemory.hpp $(srcDir)/FatalError.hpp
	$(CC) $(CFLAGS) -c $< -o $@

tuple.o: tuple.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# For canonical kd-tree
kd-tree.o: kd-tree.cpp
	$(CC) $(CFLAGS) -c kd-tree.cpp

kd: kd-tree.o HybridMemory.o tuple.o
	$(CC) $(CFLAGS) -o test_kd tuple.o kd-tree.o HybridMemory.o test_kd.cpp $(LDLIBS)

# For mult-fanout kd-tree
mfkd-tree.o: mfkd-tree.cpp
	$(CC) $(CFLAGS) -c mfkd-tree.cpp

mfkd: mfkd-tree.o HybridMemory.o tuple.o
	$(CC) $(CFLAGS) -o test_mfkd tuple.o mfkd-tree.o HybridMemory.o test_mfkd.cpp $(LDLIBS)

compare: mfkd-tree.o kd-tree.o tuple.o HybridMemory.o
	$(CC) $(CFLAGS) -o compare tuple.o kd-tree.o mfkd-tree.o HybridMemory.o compare.cpp $(LDLIBS)
	

clean: 
	rm *.o compare a.out test_kd test_mfkd 1
