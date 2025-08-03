
#include "Communication.hpp"


//std::string float2string(float f, int round = 3) {
//	std::ostringstream oss;
//	oss << std::fixed << std::setprecision(round) << f;
//	std::string str = oss.str();
//	return str;
//}

bool UDP_toolkit::init(const char* ip, u_short port) {
	server_ip = ip;
	server_port = port;

	std::cout << "Starting robot command UDP thread...\n";
	std::cout << "Target IP: " << server_ip << ":" << server_port << std::endl;


	std::cout << "Init Winsock to send command to robot...\n";
	if (WSAStartup(MAKEWORD(2, 2), &winsock) != 0) {
		std::cout << "Failed. Error Code: " << WSAGetLastError() << std::endl;
		return false;
	}
	std::cout << "succeed!\n";


	server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (server_socket == INVALID_SOCKET) {
		std::cout << "socket() failed with error code: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return false;
	}

	memset((char*)&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(server_port);


	server.sin_addr.S_un.S_addr = inet_addr(server_ip);
	if (server.sin_addr.S_un.S_addr == INADDR_NONE) {
		std::cout << "Invalid IP address: " << server_ip << std::endl;
		closesocket(server_socket);
		WSACleanup();
		return false;
	}

	//if (bind(server_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
	//	std::cout << "Bind failed with error code: " << WSAGetLastError() << std::endl;
	//	closesocket(server_socket);
	//	WSACleanup();
	//	return false;
	//

	return true;
}

void UDP_toolkit::send_data(const char* target_ip, u_short target_port) {
	sockaddr_in target;
	target.sin_family = AF_INET;
	target.sin_port = htons(target_port);
	target.sin_addr.S_un.S_addr = inet_addr(target_ip);

	sendto(server_socket, Send_Message, strlen(Send_Message), 0, (sockaddr*)&target, sizeof(target));
}

void UDP_toolkit::receive_data() {
	char buffer[1024];
	sockaddr_in from;
	int from_len = sizeof(from);

	int bytes_received = recvfrom(server_socket, Receive_Message, sizeof(Receive_Message) - 1, 0, (sockaddr*)&from, &from_len);
	if (bytes_received > 0) {
		Receive_Message[bytes_received] = '\0'; // Null-terminate the string
		//std::cout << "Received: " << Receive_Message << std::endl;
	}
}

UDP_toolkit::~UDP_toolkit() {
	closesocket(server_socket);
	WSACleanup();
}

