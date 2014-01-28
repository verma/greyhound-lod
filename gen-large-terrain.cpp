// gen-large-terrain.cpp
// Generate terrain data in C++ using the awesome libnoise
// which can be installed using brew directly from the pull request:
//
// https://raw2.github.com/krono/homebrew/7382acbeab57af7166be04e4c9afdf610cb727b4/Library/Formula/libnoise.rb
//


#include <noise/noise.h>

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


	if (terrain_size_power > 20) {
		std::cerr << "The terrain size is a power of 2 and should be less than 20." << std::endl;
		return 1;
	}

	if (cell_size_power > terrain_size_power) {
		std::cerr << "The terrain size is a power of 2 and should be less than the terrain size power." << std::endl;
		return 1;
	}


	module::Perlin myModule;
	ssize_t size = 1 << terrain_size_power;
	ssize_t cell_size = 1 << cell_size_power;

	ssize_t in_bytes = size * size * 12;
	float in_gb = in_bytes / (float) (1 << 30);

	cout << "Generating terrain data..." << std::endl;
	cout << "... " << size << "x" << size << " terrain. Full detail size: "
		<< size * size * 12 << " bytes (" << in_gb << " GB)." << std::endl;
	cout << "... Cell size is: " << cell_size << std::endl;

	size_t points_written = 0;

	float *pbuf = new float[cell_size * cell_size * 12];

	// for level 0 point generation, go trouhg the entire terrain
	//
	for (ssize_t z = -size/2 ; z < size/2 ; z += cell_size) {
		for (ssize_t x = -size/2 ; x < size/2 ; x += cell_size) {
			std::string file = file_name(x, z, cell_size, cell_size);

			std::string dst = "./data/" + file;
			if (bf::exists(dst))
				continue;	// don't compute the file unless asked to do so

			// compute date for the given cell
			//
			float *pwrite = pbuf;

			for (ssize_t j = 0 ; j < cell_size ; j ++) {
				for (ssize_t i = 0 ; i < cell_size ; i ++) {
					double y = myModule.GetValue((x+i)*MULTIPLIER, (z+j)*MULTIPLIER, 0.5);

					pwrite[0] = static_cast<float>(x+i);
					pwrite[1] = static_cast<float>(0.5 * (y + 1.0) * HEIGHT_MUL);
					pwrite[2] = static_cast<float>(z+j);

					pwrite += 3;
				}
			}

			// write it out
			//
			ofstream outfile(dst, ios::binary);
			if (!outfile.good()) {
				cerr << "Could not open file for writing..." << std::endl;
				return 1;
			}


			outfile.write(
					reinterpret_cast<char*>(pbuf), cell_size * cell_size * 12);
		}

		points_written += size * cell_size;
		if (points_written % (size * cell_size * 10) == 0 && points_written != 0) {
			std::cout << "... " << points_written << " of " << size*size
				<< " (" << 100 * points_written / (size*size) << "%)  points written."
				<< std::endl;
		}
	}
	delete [] pbuf;

	std::cout << "... Full detail grids done." << std::endl;
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

void downsample(const std::string& file, float *pdata);

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

			downsample("./data/" + tl, buf);
			out.write(reinterpret_cast<char *>(buf), downsample_size);

			downsample("./data/" + tr, buf);
			out.write(reinterpret_cast<char *>(buf), downsample_size);

			downsample("./data/" + bl, buf);
			out.write(reinterpret_cast<char *>(buf), downsample_size);

			downsample("./data/" + br, buf);
			out.write(reinterpret_cast<char *>(buf), downsample_size);

			out.close();
		}
	}

	delete [] buf;
}

// we're blind functions, cuz you got eyes
// + no filtering boys and girls, just raw pick and spit
//
void downsample(const std::string& file, float *pdata) {
	namespace bf = boost::filesystem;

	size_t in_size = bf::file_size(file);

	if (in_size % sizeof(float) != 0) {
		std::cerr << "Your file doesn't have float units?" << std::endl;
		return;
	}
		
	std::ifstream f(file, std::ios::binary);
	if (!f.good()) {
		std::cerr << "Cannot downsample: " << file << ", couldn't open." << std::endl;
		return;
	}

	// read entire contents and close
	//
	float *pin = new float[in_size / sizeof(float)];

	f.read(reinterpret_cast<char *>(pin), in_size);
	f.close();

	size_t source_size = in_size / 12;
	size_t downsample_size = source_size / 4;

	float *write_ptr = pdata;

	std::default_random_engine generator;
	std::uniform_int_distribution<size_t> distribution(0, source_size - 1);

	// go through the entire read buffer and pick every 4th point
	for (size_t i = 0 ; i < downsample_size ; i ++) {
		size_t src_i = distribution(generator);
		float *read_ptr = (pin + 3*src_i);
		
		write_ptr[0] = read_ptr[0];
		write_ptr[1] = read_ptr[1];
		write_ptr[2] = read_ptr[2];

		write_ptr += 3; // move to the beginning of next point
	}

	delete [] pin;
}
