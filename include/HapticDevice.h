#pragma once
#include <Windows.h>
#include <HD/hd.h>
#include <HDU/hduMatrix.h>
#include <HDU/hduMath.h>
#include <HDU/hduVector.h>
#include <HDU/hduError.h>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class HapticDevice {
public:
    HapticDevice()
        : hHD(HD_INVALID_HANDLE), schedulerHandle(HD_INVALID_HANDLE) {
    }

    ~HapticDevice() {
        if (schedulerHandle != HD_INVALID_HANDLE) {
            hdStopScheduler();
        }
        if (hHD != HD_INVALID_HANDLE) {
            hdDisableDevice(hHD);
        }
    }

    bool initialize(const char* dvName = HD_DEFAULT_DEVICE) {
        hHD = hdInitDevice(dvName);
		deviceName = dvName ? dvName : HD_DEFAULT_DEVICE;
        if (HD_DEVICE_ERROR(hdGetError())) {
            std::cerr << "Failed to initialize the HD device" << std::endl;
            return false;
        }
        hdMakeCurrentDevice(hHD);
        hdEnable(HD_FORCE_OUTPUT);
        
        schedulerHandle = hdScheduleAsynchronous(DeviceCallback, this, HD_DEFAULT_SCHEDULER_PRIORITY);
        return true;
    }

    void update() {
        // 可擴充：如需主動更新資料
       

    }
    
    void printDeviceInfo() {
        hdMakeCurrentDevice(hHD);

        HDdouble maxForce;
        hdGetDoublev(HD_NOMINAL_MAX_FORCE, &maxForce);
        std::cout << "Max force capability: " << maxForce << " N" << std::endl;

        HDdouble maxStiffness;
        hdGetDoublev(HD_NOMINAL_MAX_STIFFNESS, &maxStiffness);
        std::cout << "Max stiffness: " << maxStiffness << " N/mm" << std::endl;

        HDdouble updateRate;
        hdGetDoublev(HD_INSTANTANEOUS_UPDATE_RATE, &updateRate);
        std::cout << "Update rate: " << updateRate << " Hz" << std::endl;
    }

    HDint getButton1State() const {
        return button1;
    }
	HDint getButton2State() const {
		return button2;
	}

    hduVector3Dd getPosition() const {
        return currentPosition;
    }

    hduVector3Dd getRotated() const {
		return rotatedPosition;
    }

    hduVector3Dd getDeltaPosition() {
        hduVector3Dd now = currentPosition;
        hduVector3Dd delta = now - lastPosition;
        lastPosition = now;
        return delta;
    }

    void resetDeltaBase() {
        lastPosition = currentPosition;
    }

    void setForce(const hduVector3Dd& force) {
        currentForce = force;
    }

    void getTransformMatrix(hduMatrix& outMatrix) const {
        hdGetDoublev(HD_CURRENT_TRANSFORM, outMatrix);
    }

    // 取得3x3旋轉矩陣
    void getRotationMatrix(double rot[3][3]) const {
        hduMatrix mat;
        getTransformMatrix(mat);
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                rot[i][j] = mat[i][j];
    }

    // 取得四元數 (w, x, y, z)
    void getQuaternion(double quat[4]) const {
        double rot[3][3];
        getRotationMatrix(rot);
        // 轉換3x3旋轉矩陣到四元數
        double trace = rot[0][0] + rot[1][1] + rot[2][2];
        if (trace > 0.0) {
            double s = 0.5 / sqrt(trace + 1.0);
            quat[0] = 0.25 / s;
            quat[1] = (rot[2][1] - rot[1][2]) * s;
            quat[2] = (rot[0][2] - rot[2][0]) * s;
            quat[3] = (rot[1][0] - rot[0][1]) * s;
        }
        else {
            if (rot[0][0] > rot[1][1] && rot[0][0] > rot[2][2]) {
                double s = 2.0 * sqrt(1.0 + rot[0][0] - rot[1][1] - rot[2][2]);
                quat[0] = (rot[2][1] - rot[1][2]) / s;
                quat[1] = 0.25 * s;
                quat[2] = (rot[0][1] + rot[1][0]) / s;
                quat[3] = (rot[0][2] + rot[2][0]) / s;
            }
            else if (rot[1][1] > rot[2][2]) {
                double s = 2.0 * sqrt(1.0 + rot[1][1] - rot[0][0] - rot[2][2]);
                quat[0] = (rot[0][2] - rot[2][0]) / s;
                quat[1] = (rot[0][1] + rot[1][0]) / s;
                quat[2] = 0.25 * s;
                quat[3] = (rot[1][2] + rot[2][1]) / s;
            }
            else {
                double s = 2.0 * sqrt(1.0 + rot[2][2] - rot[0][0] - rot[1][1]);
                quat[0] = (rot[1][0] - rot[0][1]) / s;
                quat[1] = (rot[0][2] + rot[2][0]) / s;
                quat[2] = (rot[1][2] + rot[2][1]) / s;
                quat[3] = 0.25 * s;
            }
        }
    }

    // 取得歐拉角 (XYZ順序, 單位: 弧度)
    void getEulerAngles(double euler[3]) const {
        double rot[3][3];
        getRotationMatrix(rot);
        // 假設XYZ順序
        if (rot[0][2] < 1) {
            if (rot[0][2] > -1) {
                euler[1] = asin(rot[0][2]);
                euler[0] = atan2(-rot[1][2], rot[2][2]);
                euler[2] = atan2(-rot[0][1], rot[0][0]);
            }
            else {
                // rot[0][2] == -1
                euler[1] = -M_PI / 2;
                euler[0] = -atan2(rot[1][0], rot[1][1]);
                euler[2] = 0;
            }
        }
        else {
            // rot[0][2] == +1
            euler[1] = M_PI / 2;
            euler[0] = atan2(rot[1][0], rot[1][1]);
            euler[2] = 0;
        }
    }
    
    double* getGIMBALAngle() {
		return euler;
    }
private:
    HHD hHD;
    HDSchedulerHandle schedulerHandle;
    hduVector3Dd currentPosition{ 0,0,0 };
    hduVector3Dd lastPosition{ 0,0,0 };
    hduVector3Dd currentForce{ 0,0,0 };
    hduVector3Dd rotatedPosition;
    double euler[3]{ 0,0,0 };
    hduMatrix baseTransform;
    bool hasBase = false;
	HDint button1 = 0;
    HDint button2 = 0;
	string deviceName;

    static HDCallbackCode HDCALLBACK DeviceCallback(void* data) {
        HapticDevice* device = static_cast<HapticDevice*>(data);
		//std::cout << "Device name" << device->deviceName << std::endl;
        hdBeginFrame(device->hHD);
        hdGetDoublev(HD_CURRENT_POSITION, device->currentPosition);
        hdGetDoublev(HD_CURRENT_POSITION, device->rotatedPosition);
        hdGetDoublev(HD_CURRENT_GIMBAL_ANGLES, device->euler);
        HDint b = 0;
		device->button1 = 0;
		device->button2 = 0;
        hdGetIntegerv(HD_CURRENT_BUTTONS, &b);
        if (b & HD_DEVICE_BUTTON_2) {
			device->button2 = 1;
            hdSetDoublev(HD_CURRENT_FORCE, device->currentForce);
		}
		else {
            hdSetDoublev(HD_CURRENT_FORCE, hduVector3Dd(0, 0, 0));; // 如果沒有按下按鈕，則清除力回饋
		}
		if (b & HD_DEVICE_BUTTON_1) {
			device->button1 = 1; 
		}
        
        hdEndFrame(device->hHD);

        HDErrorInfo error;
        if (HD_DEVICE_ERROR(error = hdGetError())) {
            std::cerr << "Haptic error: " << error.errorCode << std::endl;
            return HD_CALLBACK_DONE;
        }
        return HD_CALLBACK_CONTINUE;
    }
};
