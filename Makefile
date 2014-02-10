CC=g++
PDAL_PATH=/Users/hobu/dev/git/pdal/
CFLAGS=-g -O3 -std=c++0x $(shell pkg-config --cflags libpq) -I$(PDAL_PATH)/include
LDFLAGS=-lstdc++ -lboost_filesystem -lboost_system $(shell pkg-config --libs libpq) -L$(PDAL_PATH)bin/ -lpdalcpp

TARGET=gen-large-terrain
TARGET2=fetch-terrain
TARGET3=mipmap-terrain
TARGET4=validate-points
TARGET5=query-raw
TARGET6=tail-points

all: $(TARGET2) $(TARGET3) $(TARGET4) $(TARGET5) $(TARGET6)

$(TARGET): gen-large-terrain.cpp
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $< -lnoise

$(TARGET2): fetch-terrain.cpp
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $< -lpdalcpp

$(TARGET3): mipmap-terrain.cpp
	$(CC) -o $@ $(CFLAGS) $< $(LDFLAGS)

$(TARGET4) : validate-points.cpp
	$(CC) -o $@ $(CFLAGS) $< $(LDFLAGS)

$(TARGET5) : query-raw.cpp
	$(CC) -o $@ $(CFLAGS) $< $(LDFLAGS) -lpdalcpp

$(TARGET6) : tail-points.cpp
	$(CC) -o $@ $(CFLAGS) $< $(LDFLAGS)

clean:
	rm -rf $(TARGET)
	rm -rf $(TARGET2)
	rm -rf $(TARGET3)
	rm -rf $(TARGET4)
	rm -rf $(TARGET5)
	rm -rf $(TARGET6);
