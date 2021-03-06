#include <opensor_com\Comm.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>

#ifdef _DEBUG
#define LIB_EXT "d.lib"
#else
#define LIB_EXT ".lib"
#endif
#define CV_LIB_PATH "D:/dev/staticlib64/"
#define CV_VER_NUM  CVAUX_STR(CV_MAJOR_VERSION) CVAUX_STR(CV_MINOR_VERSION) CVAUX_STR(CV_SUBMINOR_VERSION)
#pragma comment(lib, CV_LIB_PATH "opencv_core" CV_VER_NUM LIB_EXT)
#pragma comment(lib, CV_LIB_PATH "opencv_highgui" CV_VER_NUM LIB_EXT)
#pragma comment(lib, CV_LIB_PATH "opencv_imgcodecs" CV_VER_NUM LIB_EXT)
#pragma comment(lib, CV_LIB_PATH "opencv_imgproc" CV_VER_NUM LIB_EXT)
#pragma comment(lib, CV_LIB_PATH "IlmImf.lib")
#pragma comment(lib, CV_LIB_PATH "ippicvmt.lib")
#pragma comment(lib, CV_LIB_PATH "ippiw.lib")
#pragma comment(lib, CV_LIB_PATH "ittnotify.lib")
#pragma comment(lib, CV_LIB_PATH "libjasper.lib")
#pragma comment(lib, CV_LIB_PATH "libjpeg-turbo.lib")
#pragma comment(lib, CV_LIB_PATH "libpng.lib")
#pragma comment(lib, CV_LIB_PATH "libprotobuf.lib")
#pragma comment(lib, CV_LIB_PATH "libtiff.lib")
#pragma comment(lib, CV_LIB_PATH "libwebp.lib")
#pragma comment(lib, CV_LIB_PATH "zlib.lib")


//char webcamImage[720 * 1280 * 4];
//char depthFarImage[450 * 448 * 2];
//cv::Mat webcamImageMat(720, 1280, CV_8UC4, &webcamImage[0]);

#define WEBCAMBYTES		1806336
#define DEPTHFARBYTES	403200
#define DEPTHNEARBYTES	403200
#define SIDECAMERABYTES 307200

void parseError(sor::CommServer *commServer, std::string source) {
	if (commServer->connectionStatus == sor::CommServer::ConnectionStatus::CONNECTION_STATUS_CLOSED) {
		commServer->closeClientConnection();
	}
	else if (commServer->connectionStatus == sor::CommServer::ConnectionStatus::CONNECTION_STATUS_ERROR) {
		std::cout << source << ": Connection ERROR. Restarting server" << std::endl;
		delete commServer;
		cv::destroyAllWindows();
		commServer = new sor::CommServer();
		commServer->initialize();
	}
}

int main() {
	char * webcamImage = new char[WEBCAMBYTES](); //or 720x1280
	char * depthFarImage = new char[DEPTHFARBYTES]();

	cv::Mat webcamImageMat(504, 896, CV_8UC4);
	cv::Mat depthFarImageMat(450, 448, CV_16UC1);
	cv::Mat depthNearImageMat(450, 448, CV_16UC1);

	cv::Mat leftSideImageMat(480, 160, CV_8UC4);

	sor::CommServer *commServer = new sor::CommServer();
	//std::string ip_addr = "157.82.140.61";
	std::string ip_addr = "192.168.1.6";
	std::string port = "12345";

	
	std::string header;
	int counter = 0;
	bool written = false;
	char debugMessage[512];

	commServer->initialize();
	int imageNo = 0;
	while (true) { //TODO: convert to thread fetch
		char pressed = cv::waitKey(10);
		if (pressed == 27) break; //press escape
		if (pressed == 'g') written = false;

		if (commServer->ClientSocket == INVALID_SOCKET) {
			commServer->listenForClient(ip_addr.c_str(), port.c_str());
		}
		else { //receive message
			if (commServer->isConnected()) {
				commServer->getHeader(header);
				//std::cout << header << std::endl;
				if (header.compare("debug") == 0) {
					int messageSize;
					
					commServer->getMessageSize(messageSize);
					commServer->getMessage(debugMessage, messageSize);
					std::cout << "DBG>> " << debugMessage << std::endl;
				}
				else if (header.compare("webcam")==0) {
					commServer->getMessage((char *)webcamImageMat.ptr(), WEBCAMBYTES); //WHY
					parseError(commServer, "Getting webcam image: ");
					cv::imshow("rgb", webcamImageMat);
				}
				else if (header.compare("depthfar") == 0) {
					commServer->getMessage((char *)depthFarImageMat.ptr(), DEPTHFARBYTES);
					parseError(commServer, "Getting depthfar image: ");
					cv::Mat depth8bit;
					depthFarImageMat.convertTo(depth8bit, CV_8U, 1.0 / 16.0);
					cv::imshow("depthfar", depth8bit);
				}
				else if (header.compare("depthnear") == 0) {
					commServer->getMessage((char *)depthNearImageMat.ptr(), DEPTHNEARBYTES);
					parseError(commServer, "Getting depthnear image: ");
					cv::Mat depth8bit;
					depthNearImageMat.convertTo(depth8bit, CV_8U, 1.0 / 4.0);
					cv::imshow("depthnear", depth8bit);
					if (!written) {
						cv::imwrite("test" + std::to_string(imageNo++) + ".png", depthNearImageMat);
						written = true;
					}
				}
				else if (header.compare("leftside") == 0) {
					commServer->getMessage((char *)leftSideImageMat.ptr(), SIDECAMERABYTES);
					parseError(commServer, "Getting depthnear image: ");
					cv::imshow("leftside", leftSideImageMat);
				}
				else {
					std::cout << "Header not recognized. Restarting server." << std::endl;
					delete commServer;
					commServer = new sor::CommServer();
					commServer->initialize();
				}
				header = "";
			}
			parseError(commServer, "Getting header: ");
			/*else if (commServer->connectionStatus == sor::CommServer::ConnectionStatus::CONNECTION_STATUS_CLOSED){
			commServer->closeClientConnection();
			}
			else if (commServer->connectionStatus == sor::CommServer::ConnectionStatus::CONNECTION_STATUS_ERROR) {
			std::cout << "Connection ERROR. Restarting server" << std::endl;
			delete commServer;
			commServer = new sor::CommServer();
			commServer->initialize();
			}*/
		}
	}

	cv::destroyAllWindows();
	commServer->closeConnection();
	return 0;
}

//int main() {
//	char * webcamImage = new char[504 * 896 * 4](); //or 720x1280
//	char * depthFarImage = new char[448 * 450 * 2]();
//
//	cv::Mat webcamImageMat(504, 896, CV_8UC4);
//	cv::Mat depthFarImageMat(450, 448, CV_16UC1);
//
//	sor::CommServer *commServer = new sor::CommServer();
//	std::string ip_addr = "157.82.140.61";
//	std::string port = "12345";
//
//	std::string header;
//	int messageSize;
//	char debugMessage[512];
//	//char depthImage[720 * 1280 * 4];
//	int counter = 0;
//	bool written = false;
//
//	commServer->initialize();
//	while (true) { //TODO: convert to thread fetch
//		char pressed = cv::waitKey(10);
//		if (pressed == 27) break; //press escape
//
//		if (commServer->ClientSocket == INVALID_SOCKET) {
//			commServer->listenForClient(ip_addr.c_str(), port.c_str());
//		}
//		else { //receive message
//			if (commServer->isConnected()) {
//				commServer->getHeader(header, messageSize);
//				std::cout << header << " " << messageSize << std::endl;
//				if (header.compare("dbg") == 0) {
//					commServer->getMessage(debugMessage, messageSize);
//					parseError(commServer, "Getting debug message: ");
//					//std::cout << messageSize << std::endl;
//					std::cout << "DBG >> " << debugMessage << std::endl;
//				}
//				else if (header.compare("webcam") == 0) {
//					commServer->getMessage(webcamImage, 504*896*4); //WHY
//					parseError(commServer, "Getting webcam image: ");
//					//std::cout << "WEBCAM >> " << messageSize << std::endl;
//					//std::cout << webcamImage[921600] << std::endl;
//					//std::cout << webcamImageMat.at<cv::Vec4b>(360, 640) << std::endl;
//					cv::Mat argbmat(504, 896, CV_8UC4, &webcamImage[0]);
//					cv::Mat rgbamat(504, 896, CV_8UC4);
//					int from_to[] = { 0, 3, 1, 0, 2, 1, 3, 2 };
//					cv::mixChannels(&argbmat, 1, &rgbamat, 1, from_to, 4);
//					//cv::cvtColor(webcamImageMat, webcamImageMat, CV_RGBA2BGRA);
//					if (!written) {
//						cv::imwrite("test.png", rgbamat);
//						written = true;
//					}
//					cv::imshow("rgb", rgbamat);
//				}
//				else if (header.compare("depthfar") == 0) {
//					commServer->getMessage(depthFarImage, messageSize);
//					parseError(commServer, "Getting webcam image: ");
//					cv::Mat mat(450, 448, CV_16UC1, &depthFarImage[0]);
//					/*if (!written) {
//						cv::imwrite("test.png", mat);
//						written = true;
//					}*/
//					cv::Mat depth8bit;
//					mat.convertTo(depth8bit, CV_8U, 1.0 / 16.0);
//					cv::imshow("depth", depth8bit);
//				}
//				else {
//					std::cout << "Header not recognized. Restarting server." << std::endl;
//					delete commServer;
//					commServer = new sor::CommServer();
//					commServer->initialize();
//				}
//				header = "";
//			}
//			parseError(commServer, "Getting header: ");
//			/*else if (commServer->connectionStatus == sor::CommServer::ConnectionStatus::CONNECTION_STATUS_CLOSED){
//				commServer->closeClientConnection();
//			}
//			else if (commServer->connectionStatus == sor::CommServer::ConnectionStatus::CONNECTION_STATUS_ERROR) {
//				std::cout << "Connection ERROR. Restarting server" << std::endl;
//				delete commServer;
//				commServer = new sor::CommServer();
//				commServer->initialize();
//			}*/
//		}
//	}
//
//	cv::destroyAllWindows();
//	commServer->closeConnection();
//	return 0;
//}

//int main() {
//	sor::CommServer *commServer = new sor::CommServer();
//	std::string ip_addr = "157.82.140.61";
//	//std::string ip_addr = "127.0.0.1";
//	std::string port = "12345";
//	char testMessage[DEFAULTBUFFERSIZE];
//	int testMessageLength;
//
//	commServer->initialize();
//	while (true) {
//		if (commServer->ClientSocket == INVALID_SOCKET) {
//			commServer->listenForClient(ip_addr.c_str(), port.c_str());
//		}
//		else {
//			int iResult = commServer->receiveFromClient(testMessage, testMessageLength);
//			if (iResult == 0) {
//				commServer->closeClientConnection();
//			}
//			else if (iResult == SOCKET_ERROR) {
//				std::cout << "Connection ERROR. Restarting server" << std::endl;
//				delete commServer;
//				commServer = new sor::CommServer();
//				commServer->initialize();
//			}
//			else {
//				std::cout << commServer->ClientSocket << "(" << iResult << ") >> " << testMessage << std::endl;
//			}
//		}
//	}
//
//	commServer->closeConnection();
//	char test;
//	std::cin >> test;
//	return 0;
//}