#include <iostream>
#include "definition.h"
#include "Connecter.hpp"
#include "Communication.hpp"
#include "Teleoperation.hpp"
#include "ForceCalculate.hpp"
#include "GLViewer.hpp"


// OpenHaptics
#include <HL/hl.h>
#include <HD/hd.h>
#include <HDU/hduVector.h>
#include <HDU/hduError.h>
#include <HDU/hduMatrix.h>
#include <HDU/hduMath.h>

// opencv
#include <opencv2/opencv.hpp>

#include <cmath>
#include <chrono>



void udpLoop();

LRESULT CALLBACK KeyboardHookCallback(int nCode, WPARAM wParam, LPARAM lParam);
std::string vectorToString(const std::vector<double>& vec);


float INITIALTARGET_X = 350.0f;
float INITIALTARGET_Y = -60.0f;
float INITIALTARGET_Z = 263.0f;

float INITIALPOSITION_X = 400.0f;
float INITIALPOSITION_Y = -60.0f;
float INITIALPOSITION_Z = 320.0f;

float CAMERA_X = 350;
float CAMERA_Y = -60;
float CAMERA_Z = 380;

double START = clock();

OutputLayer* pOutput;

float OpenAngle = 30.0f;

int main() {


	// init bridge
	printf("[INFO] Initialize bridge \n");
	OutputLayer outputNode;
	outputNode.startTime = clock();
	pOutput = &outputNode;

	// initialize OpenHaptics
	printf("[INFO] Initialize joystick \n");
	teleoperation ITeleoperation;
	outputNode.pITeleoperation = &ITeleoperation;
	outputNode.pITeleoperation->pOutput = &outputNode;
	ITeleoperation.init(false, true);


	// Initialize UDP connection
	printf("[INFO] Initialize udp connection \n");
	UDP_toolkit server;
	outputNode.pServer = &server;
	outputNode.pServer->pOutput = &outputNode;
	server.init(TARGET_IP, TARGET_PORT);


	// Initialize OpenGL glfw
	printf("[INFO] Initialize OpenGL \n");
	GLViewer glviewer;
	glviewer.init();
	outputNode.pglviewer = &glviewer;
	outputNode.pglviewer->pOutput = &outputNode;


	// Force management
	printf("[INFO] Initialize force calculation \n");
	ForceManage IForce;
	outputNode.pForce = &IForce;
	outputNode.pForce->pOutput = &outputNode;

	// global keyboard callback
	HHOOK hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookCallback, NULL, 0);
	if (hKeyboardHook == NULL) {
		std::cerr << "Failed to install keyboard hook!" << std::endl;
		return -1;
	}

	std::thread udpThread(udpLoop);
	udpThread.detach();

	pOutput->lastFrame = clock();
	pOutput->nowFrame = clock();

	// main loop
	while (!pOutput->breakLoop) {

		pOutput->nowFrame = clock();
		glviewer.deltaTime = pOutput->nowFrame - pOutput->lastFrame;
		glviewer.frame_rate = 1.0f / (glviewer.deltaTime / CLOCKS_PER_SEC);

		char temp[512];

		try {

			IForce.tool_pose = glm::mat4{
				{ITeleoperation.RIGHT.worldPose[0][0],ITeleoperation.RIGHT.worldPose[1][0],ITeleoperation.RIGHT.worldPose[2][0],0},
				{ITeleoperation.RIGHT.worldPose[0][1],ITeleoperation.RIGHT.worldPose[1][1],ITeleoperation.RIGHT.worldPose[2][1],0},
				{ITeleoperation.RIGHT.worldPose[0][2],ITeleoperation.RIGHT.worldPose[1][2],ITeleoperation.RIGHT.worldPose[2][2],0},
				{ITeleoperation.RIGHT.worldPose[0][3],ITeleoperation.RIGHT.worldPose[1][3],ITeleoperation.RIGHT.worldPose[2][3],1}
			};

			glviewer.model_tool = IForce.tool_pose;

			// TODO calculate the force
			IForce.schedule();
			if (length(IForce.force) > 3.3f) {
				IForce.force = IForce.force / length(IForce.force) * 3.3f;
			}

			// render texts
			std::vector<std::string> texts;
			sprintf(temp, "tool position = (%f,%f,%f)", IForce.tool_pose[3][0], IForce.tool_pose[3][1], IForce.tool_pose[3][2]);
			texts.push_back(temp);
			temp[0] = '\0';

			sprintf(temp, "force = ( %f , %f , %f )", IForce.force[0], IForce.force[1], IForce.force[2]);
			texts.push_back(temp);
			temp[0] = '\0';

			sprintf(temp, "Frame rate = %f", glviewer.frame_rate);
			texts.push_back(temp);
			temp[0] = '\0';

			glviewer.setIn(texts);


			glviewer.draw();


			cv::waitKey(1);
		}
		catch (const std::out_of_range& e) {
			std::cerr << "[main] Out of range error (main 554): " << e.what() << std::endl;
		}
		catch (const std::length_error& e) {
			std::cerr << "[main] Length error: " << e.what() << std::endl;
		}
		catch (const std::invalid_argument& e) {
			std::cerr << "[main] Invalid argument error: " << e.what() << std::endl;
		}
		catch (const std::domain_error& e) {
			std::cerr << "[main] Domain error: " << e.what() << std::endl;
		}
		catch (const std::overflow_error& e) {
			std::cerr << "[main] Overflow error: " << e.what() << std::endl;
		}
		catch (const std::underflow_error& e) {
			std::cerr << "[main] Underflow error: " << e.what() << std::endl;
		}
		catch (const std::runtime_error& e) {
			std::cerr << "[main] Runtime error: " << e.what() << std::endl;
		}
		catch (const std::bad_alloc& e) {
			std::cerr << "[main] Memory allocation error: " << e.what() << std::endl;
		}
		catch (const std::bad_cast& e) {
			std::cerr << "[main] Bad cast error: " << e.what() << std::endl;
		}
		catch (cv::Exception& e) {
			const char* err_msg = e.what();
			std::cout << "[main] opencv exception caught: " << err_msg << std::endl;
		}
		catch (...) {
			std::cerr << "[main] Unknown exception caught!" << std::endl;
		}

		pOutput->lastFrame = pOutput->nowFrame;

	}


	glviewer.releaseOpenGL();

	return 0;
}

// =========================================================================


// keyboard 
LRESULT CALLBACK KeyboardHookCallback(int nCode, WPARAM wParam, LPARAM lParam) {

	if (nCode == HC_ACTION) {
		KBDLLHOOKSTRUCT* pKeyboardStruct = (KBDLLHOOKSTRUCT*)lParam;

		// check key down or key up
		if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
			// get key code or scan code
			DWORD vkCode = pKeyboardStruct->vkCode;
			DWORD scanCode = pKeyboardStruct->scanCode;

			// print message
			std::cout << "Key down - VK code: " << vkCode << ", Scan code: " << scanCode << std::endl;

			//****************
			// process input
			if (vkCode == 27) {//esc
				pOutput->breakLoop = true;
			}

			if (vkCode == 73) { // I top view
				pOutput->pglviewer->camera.moveCamera(glm::vec3(CAMERA_X, CAMERA_Y, CAMERA_Z),
													  glm::vec3(0.0f, 0.0f, -1.0f),
													  glm::vec3(1.0f, 0.0f, 0.0f),
													  glm::vec3(0.0f, 1.0f, 0.0f));
				pOutput->pglviewer->camera.Yaw = -90.0f;
				pOutput->pglviewer->camera.Pitch = 0.0f;
			}
			if (vkCode == 75) { // K front view
				pOutput->pglviewer->camera.moveCamera(glm::vec3(CAMERA_X, CAMERA_Y - 100.0f, CAMERA_Z - 50.0f),
													  glm::vec3(0.0f, 1.0f, 0.0f),
													  glm::vec3(1.0f, 0.0f, 0.0f),
													  glm::vec3(0.0f, 0.0f, 1.0f));
				pOutput->pglviewer->camera.Yaw = -90.0f;
				pOutput->pglviewer->camera.Pitch = 90.0f;
			}
			if (vkCode == 76) { // L side view
				pOutput->pglviewer->camera.moveCamera(glm::vec3(CAMERA_X + 100.0f, CAMERA_Y, CAMERA_Z - 50.0f),
													  glm::vec3(-1.0f, 0.0f, 0.0f),
													  glm::vec3(0.0f, 1.0f, 0.0f),
													  glm::vec3(0.0f, 0.0f, 1.0f));
				pOutput->pglviewer->camera.Yaw = -90.0f;
				pOutput->pglviewer->camera.Pitch = 90.0f;
			}
			if (vkCode == 82) { // r restart

				printf("[RESTART]  restart!! \n");

				pOutput->pITeleoperation->setPose(pOutput->pITeleoperation->RIGHT.rbtCurPos, pOutput->pITeleoperation->RIGHT.worldPose, INITIALPOSITION_X, INITIALPOSITION_Y, INITIALPOSITION_Z, 0, 20, 0);
				pOutput->pForce->mode = pOutput->pForce->start;


			}
			if (vkCode == 38) {// up
				//open
				OpenAngle = OpenAngle + 1.0;
				if (OpenAngle > 60) {
					OpenAngle = 60;
				}

			}
			if (vkCode == 40) {// down
				OpenAngle = OpenAngle - 1.0;
				if (OpenAngle < 0) {
					OpenAngle = 0;
				}

			}

		}
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}


void udpLoop() {

	//const int FRAME_TIME = 1000 / RUN_FPS;
	while (true) {
		//auto startTime = std::chrono::high_resolution_clock::now();

		memcpy(pOutput->pServer->Send_Message, pOutput->pITeleoperation->udpMessage, BUFFER_LEN);
		pOutput->pServer->send_data(TARGET_IP, TARGET_PORT);
		Sleep(1);

		if (pOutput->breakLoop) {
			break;
		}

	}

	printf("[Info] finish the udp loop \n");
	return;
}



std::string vectorToString(const std::vector<double>& vec) {
	std::ostringstream oss;
	for (size_t i = 0; i < vec.size(); ++i) {
		oss << vec.at(i);
		if (i != vec.size() - 1) {
			oss << ",";
		}
	}
	return oss.str();
}

