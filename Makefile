CC=gcc
CFLAGS=-g -O3 -std=c++11
LDFLAGS=-lnoise -lstdc++ -lboost_filesystem -lboost_system

TARGET=gen-large-terrain

all: $(TARGET)

$(TARGET): gen-large-terrain.cpp
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $<

clean:
	rm -rf $(TARGET)
