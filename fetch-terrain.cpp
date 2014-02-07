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

std::string file_name(ssize_t x, ssize_t z, ssize_t width, ssize_t height) {
	std::stringstream sstr;
	sstr << "data." << x << "." << z << "." << x+width << "." << z+height;
	return sstr.str();
}

void readBuffer (double minx, double miny, double maxx, double maxy, const std::string& filename, double normx, double normy, double tscale,
		size_t sizex, size_t sizey) {
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

	pdal::Dimension const& intensity = pbuf.getSchema().getDimension("Intensity");

	std::cout << "Writing to: " << filename << ", total uncropped: " << pReader->getNumPoints() << " points." << std::endl;

	std::ofstream outfile(filename, std::ios::binary);
	size_t in = 0, out = 0;
	float max_in_ = 0.0;
	for (size_t i = 0, il = pReader->getNumPoints() ; i < il ; i ++) {
		float x_ = static_cast<float>(x.applyScaling(pbuf.getField<int>(x, i))),
			  y_ = static_cast<float>(y.applyScaling(pbuf.getField<int>(y, i))),
			  z_ = static_cast<float>(z.applyScaling(pbuf.getField<int>(z, i)));

		if (x_ < minx || x_ > maxx || y_ < miny || y_ > maxy) {
			out ++;
			continue; // this point is not in our region, quick filter
		}

		float nx_ = (x_ - normx) * tscale - sizex / 2;
		float ny_ = (y_ - normy) * tscale - sizey / 2;

		/*
		//if (i % 1000 == 0) {
			std::cout << x_ << ", " << y_ << " -> " << nx_ << ", " << ny_ <<
				" norms: " << normx << ", " << normy << " tscale: " << tscale << std::endl;
		//}
		*/

		x_ = -nx_;
		y_ = ny_;

		float r_ = (float)pbuf.getField<boost::uint8_t>(r, i) / 255.0f,
			  g_ = (float)pbuf.getField<boost::uint8_t>(g, i) / 255.0f,
			  b_ = (float)pbuf.getField<boost::uint8_t>(b, i) / 255.0f;

		float in_ = (float)pbuf.getField<boost::uint16_t>(intensity, i);

		outfile.write((char*)&x_, sizeof(float));
		outfile.write((char*)&y_, sizeof(float));
		outfile.write((char*)&z_, sizeof(float));

		outfile.write((char*)&r_, sizeof(float));
		outfile.write((char*)&g_, sizeof(float));
		outfile.write((char*)&b_, sizeof(float));

		outfile.write((char*)&in_, sizeof(float));
		in++;

		max_in_ = in_ > max_in_ ? in_ : max_in_;
	}
	outfile.close();

	std::cout << "Volume point stats: inside: " << in << ", out: " << out << ", max intensity: " << max_in_ << std::endl;

	/*
	if (stage->getNumPoints() > 0)
		exit(1);
		*/
}

int main(int argc, char *argv[]) {
	int x = atoi(argv[1]);
	int y = atoi(argv[2]);
	int terrainSize = atoi(argv[3]);
	int leafNodeSize = atoi(argv[4]);

	double _xs = atof(argv[5]);
	double _xe = atof(argv[6]);

	double _ys = atof(argv[7]);
	double _ye = atof(argv[8]);

	double _tscale = atof(argv[9]);
	double _normx = atof(argv[10]);
	double _normy = atof(argv[11]);

	readBuffer(_xs, _ys, _xe, _ye, "./data/" + file_name(x, y, leafNodeSize, leafNodeSize), _normx, _normy, _tscale, terrainSize, terrainSize);
}
