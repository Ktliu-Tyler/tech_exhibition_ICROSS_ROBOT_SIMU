//#ifndef __COMMUNICATE__
#ifndef __COMMUNICATE__
#define __COMMUNICATE__


#define _USE_MATH_DEFINES
#include <math.h>
#include <cstdio>
#include <iostream> 
#include <fstream>
#include <stddef.h>
#include <iomanip>
#include <thread>
#include <winsock2.h>
//#include <signal.h>
#include <conio.h>
#include <vector>

#include <string>
#include <sstream>

#include "definition.h"

#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable:4996)

#include <opencv2/opencv.hpp>

#include "Connecter.hpp"




class UDP_toolkit {
public:
	UDP_toolkit() {};
	~UDP_toolkit();
	bool init(const char* ip, u_short port);

	void send_data(const char* target_ip, u_short target_port);
	void receive_data();

	char Receive_Message[BUFFER_LEN];
	char Send_Message[BUFFER_LEN];

	const char* server_ip;
	u_short server_port;

	OutputLayer* pOutput;
private:


	WSADATA winsock;

	sockaddr_in server;
	sockaddr_in client;
	int client_socket; //send
	int server_socket; //receive
	bool exitRequested = false;
};




#endif // !__COMMUNICATE__