// fetch-terrain.cpp
// Fetch leaf level terrain using PDAL
//

#include <pdal/pdal.hpp>
#include <pdal/PipelineManager.hpp>
#include <pdal/StageFactory.hpp>
#include <pdal/PointBuffer.hpp>
#include <pdal/StageIterator.hpp>
#include <pdal/Options.hpp>
#include <pdal/drivers/pgpointcloud/Reader.hpp>

#include <iostream>

int findPowerOfTwoLarger(double v) {
	int power = 1;
	while (static_cast<double>(1 << power) < v)
		power ++;

	return power;
}

std::string file_name(ssize_t x, ssize_t z, ssize_t width, ssize_t height) {
	std::stringstream sstr;
	sstr << "data." << x << "." << z << "." << x+width << "." << z+height;
	return sstr.str();
}

//std::shared_ptr<pdal::PointerBuffer> 
void readBuffer (double minx, double miny, double maxx, double maxy, const std::string& filename, double normx, double normy) {
	pdal::Options readerOptions;
	readerOptions.add("connection", "host='localhost' dbname='lidar' user='lidar'");
	readerOptions.add("table", "lidar");
	readerOptions.add("column", "pa");
	readerOptions.add("srid", 3857);

	char buf [4096];
	snprintf(buf, sizeof(buf) - 1,
			"PC_Intersects(pa, ST_MakeEnvelope(%lf,%lf,%lf,%lf,3857))",
			minx, miny, maxx, maxy);
	buf[sizeof(buf) - 1] = 0;
	readerOptions.add("where", buf);


	boost::shared_ptr<pdal::drivers::pgpointcloud::Reader> pReader(new
			pdal::drivers::pgpointcloud::Reader(readerOptions));

	pReader->initialize();
	std::cout << "Query: " << buf << " ::: " << pReader->getNumPoints() << " points" << std::endl;

	pdal::PointBuffer pbuf(pReader->getSchema(), pReader->getNumPoints());
	pdal::StageSequentialIterator* iterator = pReader->createSequentialIterator(pbuf);
	iterator->read(pbuf);

	//std::cout << pbuf.getSchema() << std::endl;

	// read in points
	pdal::Dimension const& x = pbuf.getSchema().getDimension("X");
	pdal::Dimension const& y = pbuf.getSchema().getDimension("Y");
	pdal::Dimension const& z = pbuf.getSchema().getDimension("Z");

	pdal::Dimension const& r = pbuf.getSchema().getDimension("Red");
	pdal::Dimension const& g = pbuf.getSchema().getDimension("Green");
	pdal::Dimension const& b = pbuf.getSchema().getDimension("Blue");


	std::cout << "Writing to: " << filename << ", total: " << pReader->getNumPoints() << " points." << std::endl;

	std::ofstream outfile(filename, std::ios::binary);
	for (size_t i = 0, il = pReader->getNumPoints() ; i < il ; i ++) {
		float x_ = static_cast<float>(x.applyScaling(pbuf.getField<int>(x, i))) - normx,
			  y_ = static_cast<float>(y.applyScaling(pbuf.getField<int>(y, i))) - normy,
			  z_ = static_cast<float>(z.applyScaling(pbuf.getField<int>(z, i)));

		float r_ = (float)pbuf.getField<boost::uint8_t>(r, i) / 255.0f,
			  g_ = (float)pbuf.getField<boost::uint8_t>(g, i) / 255.0f,
			  b_ = (float)pbuf.getField<boost::uint8_t>(b, i) / 255.0f;

		outfile.write((char*)&x_, sizeof(float));
		outfile.write((char*)&y_, sizeof(float));
		outfile.write((char*)&z_, sizeof(float));

		outfile.write((char*)&r_, sizeof(float));
		outfile.write((char*)&g_, sizeof(float));
		outfile.write((char*)&b_, sizeof(float));
	}
	outfile.close();

	/*
	if (stage->getNumPoints() > 0)
		exit(1);
		*/
}

int main(int argc, char *argv[]) {
	/*
	double minx = -10328630.5337897,
		   miny = 5261567.25756276,
		   maxx = -10293814.8637897,
		   maxy = 9433270.07756276;
		   */

	double nums[4] = { -10328630.5337897, 9400741.04756276, -10293814.8637897, 9433270.07756276 };

	double minx = nums[0],
		   miny = nums[1],
		   maxx = nums[2],
		   maxy = nums[3];

	double dimx = maxx - minx,
		   dimy = maxy - miny;

	int x = atoi(argv[1]);
	int y = atoi(argv[2]);
	int terrainSize = atoi(argv[3]);
	int leafNodeSize = atoi(argv[4]);

	int pow2 = 14;
	int leafNodePow2 = 7;

	double maxRange = std::max(dimx, dimy);

	maxRange *= 0.05;

	double centerX = minx + dimx / 2;
	double centerY = miny + dimy / 2;

	double normx = centerX - maxRange * 0.5;
	double normy = centerY - maxRange * 0.5;

	std::cout << "norms: " << normx << ", " << normy << std::endl;

	//readBuffer(-10328630.53, 5261567.25, -10298630.53, 5291567.25);

	// generate base level
	//
	//for (int y = -terrainSize / 2 ; y < terrainSize / 2 ; y += leafNodeSize) {
		//for (int x = -terrainSize / 2 ; x < terrainSize / 2 ; x += leafNodeSize) {

			double _xs = centerX + 0.5 * maxRange * x / double(terrainSize / 2);
			double _ys = centerY + 0.5 * maxRange * y / double(terrainSize / 2);

			double _xe = centerX + 0.5 * maxRange * (x + leafNodeSize) / double(terrainSize / 2);
			double _ye = centerY + 0.5 * maxRange * (y + leafNodeSize) / double(terrainSize / 2);

			//if (_xe > minx && _xs < maxx) {
				std::cout << "leafnode: " << x << ", " <<
					y << ": q: " << _xs << ", " << _ys << ", " << _xe << ", " << _ye <<
					", dims: " << _xe - _xs << ", " << _ye - _ys << std::endl;


				readBuffer(_xs, _ys, _xe, _ye, "./data/" + file_name(x, y, leafNodeSize, leafNodeSize), normx, normy);
			//}
		//}
	//}
}
