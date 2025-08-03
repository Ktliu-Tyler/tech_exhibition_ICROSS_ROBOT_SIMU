#pragma once
#ifndef __Connector__
#define __Connector__



#include <sstream>
#include <iomanip>
#include <string>
#include <direct.h> 

//#include <HL/hl.h>
//#include <HD/hd.h>
//
//#include <HDU/hduVector.h>
//#include <HDU/hduError.h>
//#include <HDU/hduMatrix.h>
//#include <HDU/hduMath.h>



class UDP_toolkit;
class ForceManage;
class teleoperation;
class GLViewer;





class OutputLayer {
public:
	OutputLayer() {

	};
	~OutputLayer() {};

	//collector* pDataCollector = NULL;
	UDP_toolkit* pServer = NULL;
	ForceManage* pForce = NULL;
	teleoperation* pITeleoperation = NULL;
	GLViewer* pglviewer = NULL;


	double startTime;
	double lastFrame;
	double nowFrame;
	bool breakLoop = false;

	std::string output_dir;
	std::string single_dirname;

	std::string float2string(float f, int round = 3) {
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(round) << f;
		std::string str = oss.str();
		return str;
	}


};


#endif
