CXX = g++
cc = gcc

LIB = -lpthread -ldl
CXXLIB = -std=c++11
BIN = sqlite sample *.o

all : $(BIN)
sqlite : sqlite3.c shell.c
	$(cc) -o $@ $^ $(LIB)
sample : Sample.cpp sqlite3.o
	$(CXX) -$(CXXLIB) -o $@ $^ $(LIB)
sqlite3.o : sqlite3.c 
	$(cc) -o $@ -c $^ $(LIB)
test :
	./sample

clean:
	rm $(BIN)

.PHONY: all, clean
