#include <opensor_com\Comm.h>

int main() {
	sor::CommClient *commClient = new sor::CommClient();
	std::string ip_addr = "157.82.140.61";
	//std::string ip_addr = "127.0.0.1";
	std::string port = "12345";
	commClient->connectToServer(ip_addr, port);
	
	char *testMsg = (char*) "Hello World!";
	commClient->sendToServer(testMsg, (int)strlen(testMsg));
	std::cout << "Sent message: " << testMsg << std::endl;
	/*int recvlen;
	char rcvMsg[DEFAULTBUFFERSIZE];
	commClient->receiveFromServer(rcvMsg, recvlen);
	std::cout << recvlen << ":Received from server: " << rcvMsg;*/
	commClient->closeConnection();

	char test;
	std::cin >> test;
	return 0;
}