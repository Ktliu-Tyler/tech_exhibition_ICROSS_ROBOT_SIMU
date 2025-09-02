#pragma once
#ifndef __DEF__
#define __DEF__
#include <glm/glm.hpp>
#include <shared_mutex>
#include "Connecter.hpp"

//#include <Eigen/Dense>

#include <omp.h>

#define RAD 0.01745329251

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// #define WINDOW_WIDTH 1600 //2500
// #define WINDOW_HEIGHT 900

#define BUFFER_LEN 512

#define TARGET_IP "127.0.0.1"//"192.168.50.111" //"127.0.0.1"
#define TARGET_PORT 5005


// teleoperation
#define SCALING_FACTOR  0.2
#define FORCE_THRES 0.01
#define HD_LEFT_DEVICE				"Left Device"
#define HD_RIGHT_DEVICE				"Right Device"


#define FORCE_THRES_SKEWLINE 1.0 //2.0
#define FORCE_MAGNITUDE_SKEWLINE 3.0//5.0 //2.0 //0.4
#define FORCE_MAGNITUDE 0.8


extern float INITIALPOSITION_X;
extern float INITIALPOSITION_Y;
extern float INITIALPOSITION_Z;

extern float INITIALTARGET_X;
extern float INITIALTARGET_Y;
extern float INITIALTARGET_Z;

extern float CAMERA_X;
extern float CAMERA_Y;
extern float CAMERA_Z;

extern float OpenAngle;


extern OutputLayer* pOutput;



#endif