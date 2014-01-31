// gen-large-terrain.cpp
// Generate terrain data in C++ using the awesome libnoise
// which can be installed using brew directly from the pull request:
//
// https://raw2.github.com/krono/homebrew/7382acbeab57af7166be04e4c9afdf610cb727b4/Library/Formula/libnoise.rb
//


#include <libnoise/noise.h>

#include <iostream>
#include <fstream>

#include <sstream>
#include <random>

#include <boost/filesystem.hpp>

#define HEIGHT_MUL 400
#define MULTIPLIER 0.001

// powers of two for terrain data size
#define DEFAULT_TERRAIN_SIZE 12			// 4096x4096
#define DEFAULT_CELL_SIZE 7				// 128x128

std::string file_name(ssize_t x, ssize_t z, ssize_t width, ssize_t height) {
	std::stringstream sstr;
	sstr << "data." << x << "." << z << "." << x+width << "." << z+height;
	return sstr.str();
}

void computeMipLevel(size_t field_size, size_t leaf_cell_size, size_t cell_size);

bool isAllDataAvailable(size_t terrainPower, size_t leafPower);

int main (int argc, char** argv)
{
	using namespace noise;
	using namespace std;

	namespace bf = boost::filesystem;

	int terrain_size_power = DEFAULT_TERRAIN_SIZE;
	int cell_size_power = DEFAULT_CELL_SIZE;

	std::cerr << argc << std::endl;
	if (argc > 1)
		terrain_size_power = atoi(argv[1]);
	if (argc > 2)
		cell_size_power = atoi(argv[2]);

	std::cout << "Terrain size: " << (1 << terrain_size_power) << ", leaf node: " << (1 << cell_size_power) << std::endl;
	std::cout << "... checking if all leaf nodes are present..." << std::endl;
	if (!isAllDataAvailable(terrain_size_power, cell_size_power)) {
		std::cout << "... NO." << std::endl;
		return 1;
	}

	std::cout << "... YES." << std::endl;

	size_t size = 1 << terrain_size_power;
	size_t cell_size = 1 << cell_size_power;

	std::cout << std::endl;

	// determine the number of mip levels
	// log4(total number of max detail cells)
	size_t mip_levels = static_cast<size_t> (log((size * size) / (cell_size * cell_size))
			/ log(4));

	std::cout << "Generating Mip levels..." << std::endl;
	std::cout << "... total " << mip_levels << " MIP levels needed." << std::endl;


	for (ssize_t i = mip_levels - 1 ; i >= 0 ; i--) {
		size_t mip_level_cell_size = cell_size << (mip_levels - i);

		std::cout << "... compute mip level: " << i << ", output cell size: "
			<< mip_level_cell_size << std::endl;

		computeMipLevel(size, cell_size, mip_level_cell_size);
	}


	// write metadata about this dataset
	//
	std::ofstream json("./data/meta.json");
	json << "{ \"terrainSize\": " << size << ", \"leafSize\": " << cell_size << " }";
	json.close();

	return 0;
}

char *downsample(const std::string& file, size_t& size);

void computeMipLevel(size_t field_size, size_t leaf_cell_size, size_t cell_size) {
	size_t source_cell_size = cell_size / 2;
	size_t downsample_size = 12 * leaf_cell_size * leaf_cell_size / 4;

	float *buf = new float[downsample_size / sizeof(float)];
	ssize_t fs = static_cast<ssize_t>(field_size);

	for (ssize_t z = -fs / 2 ; z < fs / 2; z += cell_size) {
		for (ssize_t x = -fs / 2 ; x < fs / 2 ; x += cell_size) {
			// get the name of the file we'd write down sampled data to
			//
			std::string target_file = "./data/" + file_name(x, z, cell_size, cell_size);

			// get files for four children
			//
			std::string
				tl = file_name(x, z, source_cell_size, source_cell_size),
				tr = file_name(x + source_cell_size, z, source_cell_size, source_cell_size),
				bl = file_name(x, z + source_cell_size, source_cell_size, source_cell_size),
				br = file_name(x + source_cell_size, z + source_cell_size, source_cell_size,
						source_cell_size);

			std::ofstream out(target_file, std::ios::binary);
			if (!out.good()) {
				std::cerr << "Could not write file while downsampling "
					<< target_file << std::endl;
				continue;
			}

			// note that although it'd be nice to colate data and make sure
			// the new downsampled buffer has points going left to right
			// top to bottom, it is not necessary to do so.
			//

			size_t sampleSize = 0;
			char *buf;

			// hurts to play with so much memory, it does really
			// I will fix it if it hurts even a little bit more.
			// 
			buf = downsample("./data/" + tl, sampleSize);
			if (sampleSize > 0) {
				out.write(buf, sampleSize);
				delete[] buf;
			}

			buf = downsample("./data/" + tr, sampleSize);
			if (sampleSize > 0) {
				out.write(buf, sampleSize);
				delete[] buf;
			}

			if (sampleSize > 0) {
				buf = downsample("./data/" + bl, sampleSize);
				out.write(buf, sampleSize);
				delete[] buf;
			}


			if (sampleSize > 0) {
				buf = downsample("./data/" + br, sampleSize);
				out.write(buf, sampleSize);
				delete[] buf;
			}

			out.close();
		}
	}

	delete [] buf;
}

// we're blind functions, cuz you got eyes
// + no filtering boys and girls, just raw pick and spit
//
char *downsample(const std::string& file, size_t& size) {
	namespace bf = boost::filesystem;

	size_t in_size = bf::file_size(file);

	if (in_size % sizeof(float) != 0) {
		std::cerr << "Your file doesn't have float units?" << std::endl;
		return NULL;
	}
		
	std::ifstream f(file, std::ios::binary);
	if (!f.good()) {
		std::cerr << "Cannot downsample: " << file << ", couldn't open." << std::endl;
		return NULL;
	}


	// read entire contents and close
	//
	float *pin = new float[in_size / sizeof(float)];

	f.read(reinterpret_cast<char *>(pin), in_size);
	f.close();

	size_t source_size = in_size / 24;
	size_t downsample_size = source_size / 4;

	size = 6 * downsample_size * sizeof(float);

	std::cout << "source point count: " << source_size << ", downsample size: " << downsample_size
		<< ", file size: " << size << ", div? " << size % 4 << std::endl;

	char *pdata = new char[size];
	float *write_ptr = reinterpret_cast<float*>(pdata);

	// don't you mess with ma distribution function
	std::default_random_engine generator;
	std::uniform_int_distribution<size_t> distribution(0, source_size - 1);

	// go through the entire read buffer and pick every 4th point
	for (size_t i = 0 ; i < downsample_size ; i ++) {
		size_t src_i = distribution(generator);
		float *read_ptr = (pin + 6*src_i);
		
		write_ptr[0] = read_ptr[0];
		write_ptr[1] = read_ptr[1];
		write_ptr[2] = read_ptr[2];
		write_ptr[3] = read_ptr[3];
		write_ptr[4] = read_ptr[4];
		write_ptr[5] = read_ptr[5];

		write_ptr += 6; // move to the beginning of next point
	}

	delete [] pin;
	return pdata;
}

bool isAllDataAvailable(size_t terrainPower, size_t leafPower) {
	ssize_t fs = static_cast<ssize_t>(1 << terrainPower);
	ssize_t cell_size = 1 << leafPower;

	for (ssize_t z = -fs / 2 ; z < fs / 2; z += cell_size) {
		for (ssize_t x = -fs / 2 ; x < fs / 2 ; x += cell_size) {
			// get the name of the file we'd write down sampled data to
			//
			std::string target_file = "./data/" + file_name(x, z, cell_size, cell_size);

			if (!boost::filesystem::exists(target_file))
				return false;
		}
	}

	return true;
}
