#include "Teleoperation.hpp"

double teleoperation::scalingFactor = SCALING_FACTOR;            //                         //
double teleoperation::joint6IniAng = { -46.596 };                //
double teleoperation::joint6Range[2] = { 90, 49 };               //
double teleoperation::iniToolDepth = 110;                        //
double teleoperation::iniBoundx[2] = { 313, 382 };               //                      //
double teleoperation::xIniLimit[2][2] = { { 0,355 }, {380, 700} };
double teleoperation::yIniLimit[2] = { -90, -30 };
double teleoperation::zIniLimit[2] = { 150, 300 };
double teleoperation::xBound[2] = { iniBoundx[0], iniBoundx[1] };


// calculate the transformation matrix between screen and joystick
hduMatrix RollPitchYaw2RotMat(double roll, double pitch, double yaw) {
	roll = roll * M_PI / 180;
	pitch = pitch * M_PI / 180;
	yaw = yaw * M_PI / 180;
	hduMatrix mat;

	mat[0][0] = cos(yaw) * cos(pitch);
	mat[0][1] = cos(yaw) * sin(pitch) * sin(roll) - sin(yaw) * cos(roll);
	mat[0][2] = cos(yaw) * sin(pitch) * cos(roll) + sin(yaw) * sin(roll);
	mat[0][3] = 0;

	mat[1][0] = sin(yaw) * cos(pitch);
	mat[1][1] = sin(yaw) * sin(pitch) * sin(roll) + cos(yaw) * cos(roll);
	mat[1][2] = sin(yaw) * sin(pitch) * cos(roll) - cos(yaw) * sin(roll);
	mat[1][3] = 0;

	mat[2][0] = -sin(pitch);
	mat[2][1] = cos(pitch) * sin(roll);
	mat[2][2] = cos(pitch) * cos(roll);
	mat[2][3] = 0;

	mat[3][0] = 0;
	mat[3][1] = 0;
	mat[3][2] = 0;
	mat[3][3] = 1;

	return mat;
}

void getToolAngleBetweenProbe(double* angle, hduMatrix& worldPose, double& thetaX, double& thetaY) {

	angle[0] = acos(worldPose[2][2]) * 180 / M_PI; // the angle between tool and probe in cartesian space
	angle[1] = atan2(worldPose[1][2], worldPose[0][2]) * 180 / M_PI; // angle on xy plane
	thetaX = atan2(worldPose[0][2], worldPose[2][2]) * 180 / M_PI; // angle on xz plane
	thetaY = atan2(worldPose[1][2], worldPose[2][2]) * 180 / M_PI; // angle on yz plane
}


bool isValidOrientation(hduMatrix& currPose, Joystick j) {
	int deviceID;
	bool flag = false;
	double ang[2];

	if (j._type == "Left") {
		getToolAngleBetweenProbe(ang, currPose, j.thetaX, j.thetaY);
		if (ang[0] <= 35 && ang[0] >= 0 && currPose[0][2] < 0) {
			flag = true;
		}
	}
	if (j._type == "Right") {
		getToolAngleBetweenProbe(ang, currPose, j.thetaX, j.thetaY);
		if (ang[0] <= 35 && ang[0] >= 0 && currPose[0][2] > 0) {
			double pos[3] = { currPose[0][3], currPose[1][3], currPose[2][3] };
			flag = !teleoperation::isOrientationStuck(pos, j);
		}
	}

	return flag;
}

void updateStylusOrientation(Joystick& joystick, double deltaRoll, double deltaPitch, double deltaYaw) {

	hduMatrix desiredPose;
	hduMatrix increMatrix_roll = RollPitchYaw2RotMat(deltaRoll, 0, 0);
	hduMatrix increMatrix_pitch = RollPitchYaw2RotMat(0, deltaPitch, 0);
	hduMatrix increMatrix = increMatrix_pitch.multRight(increMatrix_roll);

	desiredPose = increMatrix.multRight(joystick.worldPose);
	if (isValidOrientation(desiredPose, joystick) == true) {
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				joystick.worldPose[i][j] = desiredPose[i][j];
			}
		}
	}

}

void generateUDPword(const hduMatrix& leftMat, const hduMatrix& rightMat, const HDint& leftbtnStat, const HDint& rightbtnStat, char(&udpMessage)[512]) {
	unsigned char leftBtn[2] = { 0, 0 };
	unsigned char rightBtn[2] = { 0, 0 };
	// Parse the button states to individual button
	leftBtn[0] = ((unsigned char)leftbtnStat & 0b00000001) ? 1 : 0;
	leftBtn[1] = ((unsigned char)leftbtnStat & 0b00000010) ? 1 : 0;
	rightBtn[0] = ((unsigned char)rightbtnStat & 0b00000001) ? 1 : 0;
	rightBtn[1] = ((unsigned char)rightbtnStat & 0b00000010) ? 1 : 0;


	sprintf(udpMessage, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%u,%u,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%u,%u",
			leftMat[0][3], leftMat[1][3], leftMat[2][3], // x, y, z
			leftMat[0][0], leftMat[0][1], leftMat[0][2],
			leftMat[1][0], leftMat[1][1], leftMat[1][2],
			leftMat[2][0], leftMat[2][1], leftMat[2][2],
			leftBtn[0], leftBtn[1],
			rightMat[0][3], rightMat[1][3], rightMat[2][3], // x, y, z
			rightMat[0][0], rightMat[0][1], rightMat[0][2],
			rightMat[1][0], rightMat[1][1], rightMat[1][2],
			rightMat[2][0], rightMat[2][1], rightMat[2][2],
			rightBtn[0], rightBtn[1]);
	return;
}

//---------------------------------------------------------------------------

teleoperation* teleoperation::pThis = NULL;
teleoperation::teleoperation() {
	pThis = this;
}

void teleoperation::setPose(double* position, hduMatrix& worldPose, double x, double y, double z, double roll, double pitch, double yaw) {
	hduMatrix R;
	// Set initial Position
	worldPose.set(0, 3, x);
	worldPose.set(1, 3, y);
	worldPose.set(2, 3, z);
	position[0] = x;
	position[1] = y;
	position[2] = z;
	// Set initial Pose
	R = RollPitchYaw2RotMat(roll, pitch, yaw);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			worldPose[i][j] = R[i][j];
		}
	}
	return;
}

void teleoperation::stylusCmd2Robot(Joystick& joystick) {
	double relPos[3] = { 0, 0, 0 };
	double delta[3] = { 0,0,0 };
	hduVector3Dd jstkCurrPos;
	hduMatrix matrixRot;
	const hduMatrix matWorld2Device = RollPitchYaw2RotMat(90, 0, 0);
	int deviceID;
	HDint lastBtn = joystick.btnStat;

	deviceID = hdGetCurrentDevice();
	hdGetDoublev(HD_CURRENT_POSITION, jstkCurrPos);

	//if (deviceID == 0) {
	if (joystick._type == "Left") {
		// left
		hduVecSubtract(relPos, jstkCurrPos, joystick.jstkLastPos);
		delta[0] = relPos[0];
		delta[1] = -relPos[2];
		delta[2] = relPos[1];

		screenRotMat.multMatrixVec(hduVector3Dd(delta[0], delta[1], delta[2]), joystick.motionVec);
		hdGetIntegerv(HD_CURRENT_BUTTONS, &joystick.btnStat);
		if (joystick.btnStat == 2 || joystick.btnStat == 3) {
			joystick.rbtCurPos[0] = joystick.rbtCurPos[0] + scalingFactor * joystick.motionVec[0];
			joystick.rbtCurPos[1] = joystick.rbtCurPos[1] + scalingFactor * joystick.motionVec[1];
			joystick.rbtCurPos[2] = joystick.rbtCurPos[2] + scalingFactor * joystick.motionVec[2];
		}

		joystick.worldPose.set(0, 3, joystick.rbtCurPos[0]);
		joystick.worldPose.set(1, 3, joystick.rbtCurPos[1]);
		joystick.worldPose.set(2, 3, joystick.rbtCurPos[2]);
		hdGetDoublev(HD_CURRENT_POSITION, joystick.jstkLastPos);
		hdGetDoublev(HD_CURRENT_GIMBAL_ANGLES, joystick.currGimbalAng);
		matrixRot = RollPitchYaw2RotMat(0, 0, 180 * (joystick.currGimbalAng[2] - joystick.lastGimbalAng[2]) / M_PI);
		matrixRot.transpose();
		joystick.worldPose.multRight(matrixRot);
		hdGetDoublev(HD_CURRENT_GIMBAL_ANGLES, joystick.lastGimbalAng);
	}
	//else if (deviceID == 1) {
	if (joystick._type == "Right") {
		// right
		hduVecSubtract(relPos, jstkCurrPos, joystick.jstkLastPos);
		delta[0] = relPos[0];
		delta[1] = -relPos[2];
		delta[2] = relPos[1];

		screenRotMat.multMatrixVec(hduVector3Dd(delta[0], delta[1], delta[2]), joystick.motionVec);
		hdGetIntegerv(HD_CURRENT_BUTTONS, &joystick.btnStat);
		if (joystick.btnStat == 2 || joystick.btnStat == 3) {
			joystick.rbtCurPos[0] = joystick.rbtCurPos[0] + scalingFactor * joystick.motionVec[0];
			joystick.rbtCurPos[1] = joystick.rbtCurPos[1] + scalingFactor * joystick.motionVec[1];
			joystick.rbtCurPos[2] = joystick.rbtCurPos[2] + scalingFactor * joystick.motionVec[2];

		}

		joystick.worldPose.set(0, 3, joystick.rbtCurPos[0]);
		joystick.worldPose.set(1, 3, joystick.rbtCurPos[1]);
		joystick.worldPose.set(2, 3, joystick.rbtCurPos[2]);
		hdGetDoublev(HD_CURRENT_POSITION, joystick.jstkLastPos);
		hdGetDoublev(HD_CURRENT_GIMBAL_ANGLES, joystick.currGimbalAng);
		matrixRot = RollPitchYaw2RotMat(0, 0, 180 * (joystick.currGimbalAng[2] - joystick.lastGimbalAng[2]) / M_PI);
		matrixRot.transpose();
		joystick.worldPose.multRight(matrixRot);
		hdGetDoublev(HD_CURRENT_GIMBAL_ANGLES, joystick.lastGimbalAng);
	}

}


bool teleoperation::isOrientationStuck(double* Pos, Joystick j) {
	double x, y, z;
	double xLimit[2][2] = { {xIniLimit[0][0], xIniLimit[0][1]}, {xIniLimit[1][0], xIniLimit[1][1]} };
	double yLimit[2] = { yIniLimit[0], yIniLimit[1] };
	double zLimit[2] = { zIniLimit[0], zIniLimit[1] };
	x = Pos[0];
	y = Pos[1];
	z = Pos[2];

	if (j._type == "Left") {

		xLimit[0][1] = iniBoundx[0] + j.toolDepth * tan(abs(j.thetaX) * M_PI / 180);
		xBound[0] = xLimit[0][1];
		if (x < xLimit[0][0])
			Pos[0] = xLimit[0][0];
		else if (x > xLimit[0][1])
			Pos[0] = xLimit[0][1];

		j.toolDepth = iniToolDepth + 300 - Pos[2];

	}
	if (j._type == "Right") {

		xLimit[1][0] = iniBoundx[1] - j.toolDepth * tan(j.thetaX * M_PI / 180);
		xBound[1] = xLimit[1][0];
		if (xBound[1] >= x) {
			xBound[1] = x;
			printf("Keyboard stuck!!!\n");
			j.isStuck = true;
			return true;
		}
		else {
			j.isStuck = false;
			return false;
		}
		j.toolDepth = iniToolDepth + 300 - Pos[2];
	}


}



HDCallbackCode HDCALLBACK teleoperation::DualHapticCallback(void* data) {

	if (pThis->LEFT.isValid) {
		// if left hand is exist
		hdMakeCurrentDevice(pThis->LEFT.hHD);
		hdBeginFrame(pThis->LEFT.hHD);
		pThis->stylusCmd2Robot(pThis->LEFT);
		if (GetKeyState('W') & 0x8000) {
			std::cout << "UP detected! (L)" << std::endl;
			updateStylusOrientation(pThis->LEFT, -0.005, 0, 0);
		}
		if (GetKeyState('S') & 0x8000) {
			updateStylusOrientation(pThis->LEFT, 0.005, 0, 0);
			std::cout << "DOWN detected! (L)" << std::endl;
		}
		if (GetKeyState('A') & 0x8000) {
			updateStylusOrientation(pThis->LEFT, 0, -0.005, 0);
			std::cout << "LEFT detected! (L)" << std::endl;
		}
		if (GetKeyState('D') & 0x8000) {
			updateStylusOrientation(pThis->LEFT, 0, 0.005, 0);
			std::cout << "RIGHT detected! (L)" << std::endl;
		}
		getToolAngleBetweenProbe(pThis->LEFT.toolAng, pThis->LEFT.worldPose, pThis->LEFT.thetaX, pThis->LEFT.thetaY);
		hdEndFrame(pThis->LEFT.hHD);
	}
	if (pThis->RIGHT.isValid) {
		// if right hand is exist
		hdMakeCurrentDevice(pThis->RIGHT.hHD);
		hdBeginFrame(pThis->RIGHT.hHD);
		pThis->stylusCmd2Robot(pThis->RIGHT);
		if (GetKeyState('W') & 0x8000) {
			std::cout << "UP detected! (L)" << std::endl;
			updateStylusOrientation(pThis->RIGHT, -0.005, 0, 0);
		}
		if (GetKeyState('S') & 0x8000) {
			updateStylusOrientation(pThis->RIGHT, 0.005, 0, 0);
			std::cout << "DOWN detected! (L)" << std::endl;
		}
		if (GetKeyState('A') & 0x8000) {
			updateStylusOrientation(pThis->RIGHT, 0, -0.005, 0);
			std::cout << "LEFT detected! (L)" << std::endl;
		}
		if (GetKeyState('D') & 0x8000) {
			updateStylusOrientation(pThis->RIGHT, 0, 0.005, 0);
			std::cout << "RIGHT detected! (L)" << std::endl;
		}
		getToolAngleBetweenProbe(pThis->RIGHT.toolAng, pThis->RIGHT.worldPose, pThis->RIGHT.thetaX, pThis->RIGHT.thetaY);
		hdEndFrame(pThis->RIGHT.hHD);
	}

	generateUDPword(pThis->LEFT.worldPose, pThis->RIGHT.worldPose, pThis->LEFT.btnStat, pThis->RIGHT.btnStat, pThis->udpMessage);
	HDErrorInfo error;
	if (HD_DEVICE_ERROR(error = hdGetError())) {
		hduPrintError(stderr, &error, "Error during scheduler callback");

		if (hduIsSchedulerError(&error)) {
			return HD_CALLBACK_DONE;
		}
	}


	if (pThis->LEFT.isValid) {
		hdMakeCurrentDevice(pThis->LEFT.hHD);
		hdBeginFrame(pThis->LEFT.hHD);
		hdSetDoublev(HD_CURRENT_FORCE, hduVector3Dd{ 0,0,0 });
		hdEndFrame(pThis->LEFT.hHD);
	}

	if (pThis->LEFT.isValid) {
		hdMakeCurrentDevice(pThis->RIGHT.hHD);
		hdBeginFrame(pThis->RIGHT.hHD);
		hdSetDoublev(HD_CURRENT_FORCE, hduVector3Dd{ 0,0,0 });
		hdEndFrame(pThis->RIGHT.hHD);
	}






	return HD_CALLBACK_CONTINUE;
}

void teleoperation::init(bool _left, bool _right) {
	if (_left) {
		// left hand exit
		printf("Init left joystick \n");
		LEFT = Joystick(true);
		LEFT.toolDepth = iniToolDepth;
	}
	if (_right) {
		//right hand exit
		printf("Init right joystick \n");
		RIGHT = Joystick(false);
		RIGHT.toolDepth = iniToolDepth;

	}

	screenRotMat = RollPitchYaw2RotMat(0, 0, 0);
	double dummy_ang[2];
	setPose(LEFT.rbtCurPos, LEFT.worldPose, 300, -60, 320, 0, -20, 0);
	getToolAngleBetweenProbe(dummy_ang, LEFT.worldPose, LEFT.thetaX, LEFT.thetaY);

	setPose(RIGHT.rbtCurPos, RIGHT.worldPose, INITIALPOSITION_X, INITIALPOSITION_Y, INITIALPOSITION_Z, 0, 20, 0);
	getToolAngleBetweenProbe(dummy_ang, RIGHT.worldPose, RIGHT.thetaX, RIGHT.thetaY);

	hdStartScheduler();
	gSchedulerCallback = hdScheduleAsynchronous(DualHapticCallback, 0, HD_DEFAULT_SCHEDULER_PRIORITY);

}

Joystick::Joystick(bool _left) {
	isValid = true;
	if (_left) { // left hand
		_type = "Left";
		hHD = hdInitDevice(HD_LEFT_DEVICE);
	}
	else { // right hand
		_type = "Right";
		//hHD = hdInitDevice(HD_RIGHT_DEVICE);
		hHD = hdInitDevice(HD_DEFAULT_DEVICE);
	}


	if (HD_DEVICE_ERROR(error = hdGetError())) {
		hduPrintError(stderr, &error, "Failed to initialize the HD device");
	}



	hHLRC = hlCreateContext(hHD);
	hdMakeCurrentDevice(hHD);
	hdBeginFrame(hHD);
	hdGetDoublev(HD_CURRENT_POSITION, iniPos);
	hdGetDoublev(HD_CURRENT_GIMBAL_ANGLES, lastGimbalAng);
	hdEndFrame(hHD);


	return;

}



////////haptic


