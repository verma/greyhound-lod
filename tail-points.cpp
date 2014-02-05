// validate-points.cpp
// Given a file, make sure that all points reside in the given range as indicated by the filename
//

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdio>

#include <boost/filesystem.hpp>

void printPoint(size_t index, float vals[]) {
	std::printf("%8ld : %15.5f, %15.5f, %15.5f\n",
			index, vals[0], vals[1], vals[2]);
}

size_t num_points(const std::string& filename) {
	namespace bf = boost::filesystem;
	size_t recordSize = 24;

	return bf::file_size(filename) / recordSize;
}

bool readFile(size_t offset, size_t count, const std::string& filename) {
	std::ifstream f(filename, std::ios::binary);
	if (!f.good())
		throw std::runtime_error("Could not open file for reading");

	float record[6];

	f.seekg(sizeof(record) * offset, std::ios::beg);

	for (size_t i = 0 ; i < count && !f.eof() ; i ++) {
		f.read(reinterpret_cast<char*>(record), sizeof(record));
		printPoint(offset + i, record);
	}
	f.close();
}

int main(int argc, char *argv[]) {
	size_t offset = 0, count = 0;
	size_t pc = num_points(argv[1]);

	if (argc == 3) {
		int v = atoi(argv[2]);
		if (v > 0) {
			offset = pc - v;
			count = v;
		} else {
			offset = 0;
			count = -v;
		}
	}
	else if (argc == 4) {
		offset = atoi(argv[2]);
		count = atoi(argv[3]);
	}
	else
		throw std::runtime_error("Not sure I understand the command line");

	readFile(offset, count, argv[1]);

	return 0;
}
