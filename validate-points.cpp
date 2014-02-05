// validate-points.cpp
// Given a file, make sure that all points reside in the given range as indicated by the filename
//

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include <boost/filesystem.hpp>



std::vector<int> bounds_from_file(const std::string& filename) {
	namespace bf = boost::filesystem;

	bf::path p(filename);

	std::string f = p.filename().string();

	int l, t, r, b;
	int c = std::sscanf(f.c_str(), "data.%i.%i.%i.%i", &l, &t, &r, &b);
	assert(c == 4);

	std::vector<int> bounds;
	bounds.push_back(l);
	bounds.push_back(t);
	bounds.push_back(r);
	bounds.push_back(b);

	return bounds;
}

bool validateFile(const std::string& filename) {
	std::vector<int> bounds = bounds_from_file(filename);

	size_t size = boost::filesystem::file_size(filename);
	size_t in = 0, out = 0;
	bool allGood = true;

	if (size > 0) { // empty files are valid files
		float l = static_cast<float>(bounds[0]),
			  t = static_cast<float>(bounds[1]),
			  r = static_cast<float>(bounds[2]),
			  b = static_cast<float>(bounds[3]);

		std::cout << "File bounds: (" << l << ", " << t << ") -> (" << r << ", " << b << ")" << std::endl;

		std::ifstream f(filename, std::ios::binary);
		if (!f.good())
			return false;

		float record[6];

		while (!f.eof()) {
			f.read(reinterpret_cast<char*>(record), sizeof(record));
			float x = record[0], y = record[1], z = record[2];

			bool c = (x >= l && x <= r && y >= t && y <= b);
			if (c) in ++;
			else out ++;

			if (!c) {
				//std::cout << "FAILED: " << x << ", " << y << std::endl;
			}

			allGood &= c;
		}
		f.close();
	}

	std::cout << "Point stats: in: " << in << ", out: " << out << std::endl;
	return allGood;
}

int main(int argc, char *argv[]) {
	std::string filename(argv[1]);
	std::cout << "Checking validity for " << filename << std::endl;

	bool r = validateFile(filename);
	std::cout << "    " << (r ? "VALID" : "NOT-VALID") << std::endl;

	return r ? 0 : 1;
}
