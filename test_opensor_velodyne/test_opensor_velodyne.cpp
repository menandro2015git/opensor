// test_opensor_velodyne.cpp : Defines the entry point for the console application.
//
#include <opensor_velodyne/Velodyne.h>
#include <stdio.h>
//#include <boost/array.hpp>
//#include <boost/asio.hpp>
//#include <boost/system/config.hpp>

#include <opencv2/viz.hpp>
#include <opencv2/opencv.hpp>

#define CV_LIB_PATH "D:/dev/lib64/"
#define LIB_EXT ".lib"
#define CV_VER_NUM  CVAUX_STR(CV_MAJOR_VERSION) CVAUX_STR(CV_MINOR_VERSION) CVAUX_STR(CV_SUBMINOR_VERSION)

#pragma comment(lib, CV_LIB_PATH "opencv_core" CV_VER_NUM LIB_EXT)
#pragma comment(lib, CV_LIB_PATH "opencv_highgui" CV_VER_NUM LIB_EXT)
#pragma comment(lib, CV_LIB_PATH "opencv_viz" CV_VER_NUM LIB_EXT)

//#pragma comment(lib, "D:/dev/staticlib/boost_1_67_0/stage/lib/libboost_system-vc141-mt-x64-1_67.lib")

void test_pcap_withviz() {
	sor::Velodyne *velodyne = new sor::Velodyne("H:/data_vrst2018_proc/iis07172018.pcap");

	if (!velodyne->isOpen()) {
		std::cout << "Can't open velodyne." << std::endl;
		return;
	}

	cv::viz::Viz3d viewer("Velodyne");

	// Register Keyboard Callback
	viewer.registerKeyboardCallback(
		[](const cv::viz::KeyboardEvent& event, void* cookie) {
		// Close Viewer
		if (event.code == 'q' && event.action == cv::viz::KeyboardEvent::Action::KEY_DOWN) {
			static_cast<cv::viz::Viz3d*>(cookie)->close();
		}
	}
		, &viewer
		);

	//char pressed = ' ';
	std::vector<cv::Vec3f> *buffer = new std::vector<cv::Vec3f>();

	while (velodyne->isRunning() && (!viewer.wasStopped())) {
		char pressed = cv::waitKey(10);
		if (pressed == 27) break; //not working

								  //std::cout << "test" << std::endl;
		std::vector<sor::Laser> lasers;
		//velodyne->retrieve(lasers, false);
		*velodyne >> lasers;

		if (lasers.empty()) {
			continue;
		}
		// Convert to 3-dimension Coordinates
		//std::vector<cv::Vec3f> buffer(lasers.size());
		//std::cout << "Lasers: " << lasers.size() << std::endl;
		for (const sor::Laser& laser : lasers) {
			const double distance = static_cast<double>(laser.distance) * 0.002;
			const double azimuth = laser.azimuth  * CV_PI / 180.0;
			const double vertical = laser.vertical * CV_PI / 180.0;

			float x = static_cast<float>((distance * std::cos(vertical)) * std::sin(azimuth));
			float y = static_cast<float>((distance * std::cos(vertical)) * std::cos(azimuth));
			float z = static_cast<float>((distance * std::sin(vertical)));

			//std::cout << distance << " "  << azimuth << " " << vertical << std::endl;

			if ((x == 0.0f && y == 0.0f && z == 0.0f)) {
				x = std::numeric_limits<float>::quiet_NaN();
				y = std::numeric_limits<float>::quiet_NaN();
				z = std::numeric_limits<float>::quiet_NaN();
			}

			buffer->push_back(cv::Vec3f(x, y, z));
		}

		// Create Widget
		cv::Mat cloudMat = cv::Mat(buffer->size(), 1, CV_32FC3, buffer->data());
		cv::viz::WCloud cloud(cloudMat);

		cv::viz::writeCloud("output.ply", cloudMat);

		// Show Point Cloud
		viewer.showWidget("Cloud", cloud);
		viewer.spinOnce();

		//std::cout << "size: " << buffer->size() << std::endl;
		buffer->clear();
	}
	// Close All Viewers
	cv::viz::unregisterAllWindows();

	std::cout << "Closing..." << std::endl;
}

void test_withviz() {
	sor::Velodyne *velodyne = new sor::Velodyne("157.82.140.61", 2368);

	if (!velodyne->isOpen()) {
		std::cout << "Can't open velodyne." << std::endl;
		return;
	}

	cv::viz::Viz3d viewer("Velodyne");

	// Register Keyboard Callback
	viewer.registerKeyboardCallback(
		[](const cv::viz::KeyboardEvent& event, void* cookie) {
		// Close Viewer
		if (event.code == 'q' && event.action == cv::viz::KeyboardEvent::Action::KEY_DOWN) {
			static_cast<cv::viz::Viz3d*>(cookie)->close();
		}
	}
		, &viewer
		);

	//char pressed = ' ';
	std::vector<cv::Vec3f> *buffer = new std::vector<cv::Vec3f>();

	while (velodyne->isRunning() && (!viewer.wasStopped())) {
		char pressed = cv::waitKey(10);
		if (pressed == 27) break; //not working

								  //std::cout << "test" << std::endl;
		std::vector<sor::Laser> lasers;
		//velodyne->retrieve(lasers, false);
		*velodyne >> lasers;

		if (lasers.empty()) {
			continue;
		}
		// Convert to 3-dimension Coordinates
		//std::vector<cv::Vec3f> buffer(lasers.size());
		//std::cout << "Lasers: " << lasers.size() << std::endl;
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

			buffer->push_back(cv::Vec3f(x, y, z));
		}

		// Create Widget
		cv::Mat cloudMat = cv::Mat(buffer->size(), 1, CV_32FC3, buffer->data());
		cv::viz::WCloud cloud(cloudMat);

		// Show Point Cloud
		viewer.showWidget("Cloud", cloud);
		viewer.spinOnce();

		//std::cout << "size: " << buffer->size() << std::endl;
		buffer->clear();
	}
	// Close All Viewers
	cv::viz::unregisterAllWindows();

	std::cout << "Closing..." << std::endl;
}

void test_tcp_withviz() {
	sor::Velodyne *velodyne = new sor::Velodyne("192.168.1.102", 13000, sor::CaptureMode::TCP);
	
	if (!velodyne->isOpen()) {
		std::cout << "Can't open velodyne." << std::endl;
		return;
	}
	
	cv::viz::Viz3d viewer("Velodyne");

	// Register Keyboard Callback
	viewer.registerKeyboardCallback(
		[](const cv::viz::KeyboardEvent& event, void* cookie) {
		// Close Viewer
		if (event.code == 'q' && event.action == cv::viz::KeyboardEvent::Action::KEY_DOWN) {
			static_cast<cv::viz::Viz3d*>(cookie)->close();
		}
	}
		, &viewer
		);

	//char pressed = ' ';
	std::vector<cv::Vec3f> *buffer = new std::vector<cv::Vec3f>();
	
	while (velodyne->isRunning() && (!viewer.wasStopped())) {
		char pressed = cv::waitKey(10);
		if (pressed == 27) break; //not working

								  //std::cout << "test" << std::endl;
		std::vector<sor::Laser> lasers;
		//velodyne->retrieve(lasers, false);
		*velodyne >> lasers;

		if (lasers.empty()) {
			continue;
		}
		// Convert to 3-dimension Coordinates
		//std::vector<cv::Vec3f> buffer(lasers.size());
		//std::cout << "Lasers: " << lasers.size() << std::endl;
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

			buffer->push_back(cv::Vec3f(x, y, z));
		}

		// Create Widget
		cv::Mat cloudMat = cv::Mat(buffer->size(), 1, CV_32FC3, buffer->data());
		cv::viz::WCloud cloud(cloudMat);

		// Show Point Cloud
		viewer.showWidget("Cloud", cloud);
		viewer.spinOnce();

		//std::cout << "size: " << buffer->size() << std::endl;
		buffer->clear();
	}
	// Close All Viewers
	cv::viz::unregisterAllWindows();

	std::cout << "Closing..." << std::endl;
}

int main()
{
	test_tcp_withviz();
	return 0;
}

