#include <opensor_com\Comm.h>
#include <opencv2/opencv.hpp>

#ifdef _DEBUG
#define LIB_EXT "d.lib"
#else
#define LIB_EXT ".lib"
#endif
#define CV_LIB_PATH "D:/dev/lib64/"
#define CV_VER_NUM  CVAUX_STR(CV_MAJOR_VERSION) CVAUX_STR(CV_MINOR_VERSION) CVAUX_STR(CV_SUBMINOR_VERSION)
#pragma comment(lib, CV_LIB_PATH "opencv_core" CV_VER_NUM LIB_EXT)
#pragma comment(lib, CV_LIB_PATH "opencv_highgui" CV_VER_NUM LIB_EXT)


int main() {
	sor::CommServer *commServer = new sor::CommServer();
	std::string ip_addr = "157.82.140.61";
	//std::string ip_addr = "127.0.0.1";
	std::string port = "12345";
	char testMessage[DEFAULTBUFFERSIZE];
	int testMessageLength;

	commServer->initialize();
	while (true) {
		if (commServer->ClientSocket == INVALID_SOCKET) {
			commServer->listenForClient(ip_addr.c_str(), port.c_str());
		}
		else {
			int iResult = commServer->receiveFromClient(testMessage, testMessageLength);
			std::cout << iResult << std::endl;
			if (iResult == 0) {
				commServer->closeClientConnection();
			}
			else if (iResult == SOCKET_ERROR) {
				std::cout << "Connection ERROR. Restarting server" << std::endl;
				delete commServer;
				commServer = new sor::CommServer();
				commServer->initialize();
			}
			else {
				std::cout << "Client (" << commServer->ClientSocket << "): " << testMessage << std::endl;
			}
		}
	}

	/*char testMessage[DEFAULTBUFFERSIZE];
	int testMessageLength;*/
	//commServer->receiveFromClient(testMessage, testMessageLength);
	//std::cout << testMessageLength << " :Received: " << testMessage << std::endl;
	//commServer->sendToClient(testMessage, testMessageLength);

	commServer->closeConnection();
	char test;
	std::cin >> test;
	return 0;
}