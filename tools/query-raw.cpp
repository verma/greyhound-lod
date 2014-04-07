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

bool closeToZero(double v) {
	return std::abs(v) < 0.000001;
}

bool hasZeros (double minx, double miny, double maxx, double maxy, double normx, double normy) {
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

	size_t origin_points = 0;
	for (size_t i = 0, il = pReader->getNumPoints() ; i < il ; i ++) {
		float x_ = static_cast<float>(x.applyScaling(pbuf.getField<int>(x, i))) - normx,
			  y_ = static_cast<float>(y.applyScaling(pbuf.getField<int>(y, i))) - normy,
			  z_ = static_cast<float>(z.applyScaling(pbuf.getField<int>(z, i)));



		if (closeToZero(x_) &&
			closeToZero(y_)) {
				origin_points ++;
		}

	}
	std::cout << "Volume point stats: origin: " << origin_points << std::endl;
	return origin_points > 0;
}

int main(int argc, char *argv[]) {
	double _xs = atof(argv[1]);
	double _xe = atof(argv[2]);

	double _ys = atof(argv[3]);
	double _ye = atof(argv[4]);

	double _normx = atof(argv[5]);
	double _normy = atof(argv[6]);

	return hasZeros(_xs, _ys, _xe, _ye, _normx, _normy) ? 1 : 0;
}
