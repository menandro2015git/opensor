// test_opensor_velodyne_noviz.cpp : Defines the entry point for the console application.
//

#include <opensor_viewer/Viewer.h>
#include <opensor_velodyne/Velodyne.h>
#include <stdio.h>

int main()
{
	sor::Viewer *viewer = new sor::Viewer();
	viewer->createWindow(800, 600, "test");
	viewer->run();

	sor::Velodyne *velodyne = new sor::Velodyne("157.80.140.28", 2368);
	if (!velodyne->isOpen()) {
		std::cout << "Can't open velodyne." << std::endl;
		return 0;
	}

	sor::CgObject *pointCloud = new sor::CgObject();
	pointCloud->loadShader("pointshader.vert", "pointshader.frag");
	pointCloud->setDrawMode(sor::CgObject::Mode::POINTS);

	std::vector<float> *vertexArray = new std::vector<float>();
	std::vector<unsigned int> *indexArray = new std::vector<unsigned int>();

	while (velodyne->isRunning()) {
		std::vector<sor::Laser> lasers;
		//velodyne->retrieve(lasers, false);
		*velodyne >> lasers;

		if (lasers.empty()) {
			continue;
		}

		// Create index array
		int ncols = lasers.size() / 16;
		int nrows = 16;

		for (int col = 0; col < ncols - 1; col++) {
			for (int row = 0; row < nrows; row++) {
				// Upper triangle
				if (row < nrows - 1) {
					indexArray->push_back(nrows*col + row);
					indexArray->push_back(nrows*col + row + 1);
					indexArray->push_back(nrows*(col + 1) + row);
				}
				
				// Lower triangle
				if ((row >= 1) && (row < nrows)) {
					indexArray->push_back(nrows * col + row + 1);
					indexArray->push_back(nrows*(col + 1) + row);
					indexArray->push_back(nrows*(col + 1) + row + 1);
				}
			}
			
		}

		// Convert to 3-dimension Coordinates
		for (const sor::Laser& laser : lasers) {
			const double distance = static_cast<double>(laser.distance);
			const double azimuth = laser.azimuth  * CV_PI / 180.0;
			const double vertical = laser.vertical * CV_PI / 180.0;

			float x = static_cast<float>((distance * std::cos(vertical)) * std::sin(azimuth));
			float y = static_cast<float>((distance * std::cos(vertical)) * std::cos(azimuth));
			float z = static_cast<float>((distance * std::sin(vertical)));

			//std::cout << distance << " "  << azimuth << " " << vertical << std::endl;

			if ((x == 0.0f && y == 0.0f && z == 0.0f) || (distance < 200.0)) {
				x = std::numeric_limits<float>::quiet_NaN();
				y = std::numeric_limits<float>::quiet_NaN();
				z = std::numeric_limits<float>::quiet_NaN();
			}

			vertexArray->push_back(x);
			vertexArray->push_back(y);
			vertexArray->push_back(z);
		}

		pointCloud->loadData(*vertexArray, *indexArray, sor::CgObject::ArrayFormat::VERTEX_TEXTURE);

		indexArray->clear();
		vertexArray->clear();
	}

	viewer->close();

    return 0;
}


