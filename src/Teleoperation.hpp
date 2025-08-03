#ifndef __teleoperation__
#define __teleoperation__
#pragma once
#ifdef  _WIN64
#pragma warning (disable:4996)
#endif
#define _USE_MATH_DEFINES
#include <cstdio>
#include <iostream> 
#include <fstream>
#include <string>
#include <iomanip>
#include <math.h>
#include <thread>
#include <winsock2.h>
#include <signal.h>

#include "definition.h"
#include "Connecter.hpp"


#if defined(WIN32)
# include <conio.h>
#else
# include "conio.h"
#endif

#include <HL/hl.h>
#include <HD/hd.h>

#include <HDU/hduVector.h>
#include <HDU/hduError.h>
#include <HDU/hduMatrix.h>
#include <HDU/hduMath.h>




#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Geometry> 

//using namespace std;
#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable:4996)



class Joystick {
public:
	Joystick() : isValid(false) {};
	Joystick(bool _left);


	HHD hHD;
	HHLRC hHLRC;
	std::string _type = "Left"; // or right
	HDErrorInfo error;

	HDint btnStat;
	hduMatrix worldPose;
	hduVector3Dd iniPos;
	HDdouble lastGimbalAng[3];
	HDdouble currGimbalAng[3];
	hduVector3Dd jstkLastPos;
	double rbtCurPos[3];
	double toolAng[2];
	double toolDepth;

	hduVector3Dd motionVec;

	double thetaX;
	double thetaY;

	bool isValid;
	bool isStuck = false;

	hduVector3Dd resultantForce;

private:
};

class teleoperation {
public:

	teleoperation();

	void init(bool _left, bool _right);

	Joystick LEFT;
	Joystick RIGHT;

	char udpMessage[BUFFER_LEN];
	hduMatrix screenRotMat;


	HDSchedulerHandle gSchedulerCallback = HD_INVALID_HANDLE;
	static HDCallbackCode HDCALLBACK DualHapticCallback(void* data);

	static teleoperation* pThis;

	// set the arm pose
	void setPose(double* position, hduMatrix& worldPose, double x, double y, double z, double roll, double pitch, double yaw);
	// read joystick pose and update the pose of robot
	void stylusCmd2Robot(Joystick& joystick);

	static bool isOrientationStuck(double* Pos, Joystick j);


	static double scalingFactor;            //                         //
	static double joint6IniAng;                //
	static double joint6Range[2];               //
	static double iniToolDepth;                        //
	static double iniBoundx[2];               //                      //
	static double xIniLimit[2][2];
	static double yIniLimit[2];
	static double zIniLimit[2];
	static double xBound[2];



	OutputLayer* pOutput = NULL;
private:



};



#endif
