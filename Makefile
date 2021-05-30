CC=g++
CFLAGS=-std=c++17 -Wall -ggdb -o test -D_FILE_OFFSET_BITS=64 -pthread 
OBJ = test 

all: targets

targets: test

clean: 
	rm test

test: 
	$(CC) $(CFLAGS) test.cc /usr/local/lib/libredis++.a /usr/local/lib/libhiredis.a $(pkg-config fuse3 --cflags --libs)
	
