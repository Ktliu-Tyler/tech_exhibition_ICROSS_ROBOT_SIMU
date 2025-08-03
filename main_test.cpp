//#define GLM_ENABLE_EXPERIMENTAL
//#include <glm/gtx/matrix_decompose.hpp>
//#include <GL/glew.h>
//#include <GLFW/glfw3.h>
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>
//#include <btBulletDynamicsCommon.h>
//#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
//#include <iostream>
//#include <memory>
//#include <thread>
//#include <chrono>
//#include <iomanip>
//
//// my lib
//#include "shader.h"
//#include "Model_new.h"
//#include "Forcep_new.h"
//#include "Tool.h"
//#include "HapticDevice.h"
//#include "UIManager.h"
//#include "Constant.h"
//#include "GameEngine.h"
//
//// 輸入你的兩隻裝置序號
//const char* SERIAL1 = "21069000752";
//const char* SERIAL2 = "21069000751";
//
//int main() {
//    HHD hHD1 = hdInitDevice(SERIAL1);
//    if (hHD1 == HD_INVALID_HANDLE) {
//        std::cout << "Failed to init device 1" << std::endl;
//        return -1;
//    }
//    HHD hHD2 = hdInitDevice(SERIAL2);
//    if (hHD2 == HD_INVALID_HANDLE) {
//        std::cout << "Failed to init device 2" << std::endl;
//        return -1;
//    }
//    hdStartScheduler();
//
//    std::cout << "Device 1 serial: " << hdGetString(HD_DEVICE_SERIAL_NUMBER) << std::endl;
//    hdMakeCurrentDevice(hHD2);
//    std::cout << "Device 2 serial: " << hdGetString(HD_DEVICE_SERIAL_NUMBER) << std::endl;
//
//    for (int i = 0; i < 1000; ++i) {
//        // 讀 device 1
//        hdMakeCurrentDevice(hHD1);
//        hdBeginFrame(hHD1);
//        double pos1[3];
//        hdGetDoublev(HD_CURRENT_POSITION, pos1);
//        int btn1 = 0;
//        hdGetIntegerv(HD_CURRENT_BUTTONS, &btn1);
//        hdEndFrame(hHD1);
//
//        // 讀 device 2
//        hdMakeCurrentDevice(hHD2);
//        hdBeginFrame(hHD2);
//        double pos2[3];
//        hdGetDoublev(HD_CURRENT_POSITION, pos2);
//        int btn2 = 0;
//        hdGetIntegerv(HD_CURRENT_BUTTONS, &btn2);
//        hdEndFrame(hHD2);
//
//        std::cout << "1: (" << pos1[0] << ", " << pos1[1] << ", " << pos1[2] << ") B1: "
//            << ((btn1 & HD_DEVICE_BUTTON_1) ? 1 : 0) << " B2: " << ((btn1 & HD_DEVICE_BUTTON_2) ? 1 : 0);
//        std::cout << " | 2: (" << pos2[0] << ", " << pos2[1] << ", " << pos2[2] << ") B1: "
//            << ((btn2 & HD_DEVICE_BUTTON_1) ? 1 : 0) << " B2: " << ((btn2 & HD_DEVICE_BUTTON_2) ? 1 : 0) << std::endl;
//
//        std::this_thread::sleep_for(std::chrono::milliseconds(20));
//    }
//    hdStopScheduler();
//    hdDisableDevice(hHD1);
//    hdDisableDevice(hHD2);
//    return 0;
//}
