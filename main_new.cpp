#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <iomanip>
#include <winsock2.h>

#include "Communication.hpp"

// my lib
#include "shader.h"
#include "Model_new.h"
#include "Forcep_new.h"
#include "Tool.h"
#include "HapticDevice.h"
#include "UIManager.h"
#include "Constant.h"
#include "iCROSS.h"

const unsigned int SCR_WIDTH = WIDTH;
const unsigned int SCR_HEIGHT = HEIGHT;
unsigned int ACTUAL_WIDTH = SCR_WIDTH;
unsigned int ACTUAL_HEIGHT = SCR_HEIGHT;

GameState currentState = GameState::MENU;
Camera camera(CAMERA_POSITION);
UDP_toolkit* pServer = new UDP_toolkit;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool mouseCONTROL = MOUSE_INVISIBLE; // 是否啟用滑鼠控制攝像機
bool InstructionON = SHOW_INSTRUCTION; // 是否顯示控制說明
bool hapticTESTMODE = HAPTIC_MODE; // 是否啟用觸覺測試模式
static bool tPressed = false;
static bool rPressed = false;
static bool f1Pressed = false;
static bool success = false;
static bool catched = false;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool firstMouse = true;

float WOUND_MESH[4] = { -1.4f, 1.3f, -1.5f, 2.35f };

iCROSS* leftArm = nullptr;
iCROSS* rightArm = nullptr;
Shader* ourShader = nullptr;
ForcepControl forcep1Control;
ForcepControl forcep2Control; 

glm::vec3 HITPOINT = glm::vec3(0.0f, 0.0f, 0.0f); 
glm::vec3 ELLOPSEPOINT = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 CHECK_UPPERPOINT = glm::vec3(0.0f, 0.0f, 0.0f); 
glm::vec3 CHECK_LOWERPOINT = glm::vec3(0.0f, 0.0f, 0.0f); 
glm::vec3 CHECK_BODY = glm::vec3(0.0f, 0.0f, 0.0f); 
glm::vec3 CHECK_VESSEL = glm::vec3(0.0f, 0.0f, 0.0f); 
glm::vec3 NORMAL_DIR = glm::vec3(0.0f, 0.0f, 0.0f);

std::shared_ptr<Forcep> forcep1;
std::shared_ptr<Forcep> forcep2;
std::shared_ptr<Model> wound;
std::shared_ptr<Model> vessel;
std::shared_ptr<Model> vessel_squeeze;
std::shared_ptr<Model> clip;

static btDefaultCollisionConfiguration collisionConfig;
static btCollisionDispatcher dispatcher(&collisionConfig);
static btDbvtBroadphase broadphase;
static btCollisionWorld collisionWorld(&dispatcher, &broadphase, &collisionConfig);
static float lastCollisionTime = 0.0f;
static bool forceEnabled = true;
static float fpsTimer = 0.0f;
static int frameCount = 0;
static float displayFPS = 0.0f;

const double LINK_PARAM_L[5] = { 91.5, 155.9, 24.5, 220, 520 };
const double LINK_PARAM_R[5] = { 91.5, 155.9, 24.5, 220, 520 };
const double JOINT_LIMIT_L[6][2] = { {0, 350}, {0, 150}, {-90, 0}, {-90, 90}, {-60, 60}, {-360, 360} };
const double JOINT_LIMIT_R[6][2] = { {350, 700}, {0, 150}, {0, 90}, {-90, 90}, {-60, 60}, {-360, 360} };
const double INI_VALUE_L[6] = { 0, 0, 0, 0, 0, 0 };
const double INI_VALUE_R[6] = { 700, 0, 0, 0, 0, 0 };
const double FORCE_SCALING_FACTOR = 0.6;
const int MASK_SIZE_BIG = 500;
const int MASK_SIZE_SMALL = 10;

int CatchedCheckForcep1();
int CatchedCheckForcep2();
float CheckWoundMesh();
float distanceToEllipseXZ(const glm::vec3& point, float a, float b, float cx, float cz);
float distanceToEllipsoidSurface(const glm::vec3& point, const glm::vec3& center, float a, float b, float c);
bool init();
void CaculateFPS();
void GameModeCheck();
void moveWOUND();
void updateController();
void updateMesh();
void updateCollision();
void UIupdate();
void renderUI();
void render();
void printMenu();
void Checkcatched_stated();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void forcepKeyboard(GLFWwindow* window);
void processInput(GLFWwindow* window);
void checkCollisionWithWound(btCollisionWorld& world, const std::vector<std::shared_ptr<Forcep>>& forceps, Model* wound);
void checkCollisionWithVessel(btCollisionWorld& world, const std::vector<std::shared_ptr<Forcep>>& forceps, Model* vessel);
void checkCollisionForceps(btCollisionWorld& world, const std::vector<std::shared_ptr<Forcep>>& forceps);
void udpLoop();

std::string generateRobotCommand(const std::shared_ptr<Forcep>& forcep1, const std::shared_ptr<Forcep>& forcep2);
hduMatrix getForcepWorldMatrix(glm::vec3 pos, glm::mat3 R);
GLFWwindow* window = nullptr;
std::unique_ptr<UIManager> uiManager;


int main() {
    if (!init()) {
        return -1;
    }
    while (!glfwWindowShouldClose(window)) {
		GameModeCheck();
        CaculateFPS();
        moveWOUND();
        processInput(window);
        updateController();
        UIupdate();
        render();
        updateMesh();
        updateCollision();
        renderUI();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}



bool init() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if (FULL_SCREEN) {
        // 全螢幕模式
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        ACTUAL_WIDTH = mode->width;
        ACTUAL_HEIGHT = mode->height;
        window = glfwCreateWindow(ACTUAL_WIDTH, ACTUAL_HEIGHT, "Tech Exhibition", monitor, NULL);
    }
    else {
        // 視窗模式
        ACTUAL_WIDTH = SCR_WIDTH;
        ACTUAL_HEIGHT = SCR_HEIGHT;
        window = glfwCreateWindow(ACTUAL_WIDTH, ACTUAL_HEIGHT, "Tech Exhibition", NULL, NULL);
    }

    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    pServer->init(TARGET_IP, TARGET_PORT);

    // Send UDP
    std::thread udpThread(udpLoop);
    udpThread.detach();

    // 更新滑鼠位置初始值
    lastX = ACTUAL_WIDTH / 2.0f;
    lastY = ACTUAL_HEIGHT / 2.0f;

    // 設定滑鼠模式
    if (mouseCONTROL) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    // 切換到美式英文鍵盤
    HKL hkl = LoadKeyboardLayout(L"00000409", KLF_ACTIVATE);
    ActivateKeyboardLayout(hkl, KLF_SETFORPROCESS);

    // 初始化 GLEW
    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return false;
    }

    // 配置 OpenGL
    glEnable(GL_DEPTH_TEST);

    // Initialize UI Manager with actual screen size
    uiManager = std::make_unique<UIManager>(ACTUAL_WIDTH, ACTUAL_HEIGHT);
    if (!uiManager->initialize()) {
        std::cout << "Failed to initialize UI Manager" << std::endl;
        return false;
    }

    // Load shaders
    ourShader = new Shader("vertex_shader.glsl", "fragment_shader.glsl");
    forcep1 = std::make_shared<Forcep>(glm::vec3(-2.0f, 3.0f, 0.0f), glm::vec3(0.0f, 0.0f, -70.0f), 0.05f);
    forcep2 = std::make_shared<Forcep>(glm::vec3(2.0f, 3.0f, 0.0f), glm::vec3(0.0f, 0.0f, -110.0f), 0.05f);


    if (hapticTESTMODE) {
        // Enable haptic test mode
        
        if (!ONE_HAPTIC) {
            if (!forcep2->haptic->initialize("Right Device")) {
                std::cout << "Haptic device initialization failed R" << std::endl;
                return false;
            }
            if (!forcep1->haptic->initialize("Left Device")) {
                std::cout << "Haptic device initialization failed L" << std::endl;
                return false;
            }
		}
		else {
			if (!forcep1->haptic->initialize("Default Device")) {
				std::cout << "Haptic device initialization failed" << std::endl;
				return false;
			}
		}
        hdStartScheduler();

    }
    uiManager->updateMode(u8"展示模式");
    //uiManager->showWelcomeScreen();


    // 設定夾子的鉗爪偏移（根據實際模型調整）
    forcep1->num = 1;
    forcep2->num = 2;
    forcep1->setClawUpperOffset(glm::vec3(0.0f, 0.0f, 0.0f));
    forcep1->setClawLowerOffset(glm::vec3(0.0f, 0.0f, 0.0f));
    forcep2->setClawUpperOffset(glm::vec3(0.0f, 0.0f, 0.0f));
    forcep2->setClawLowerOffset(glm::vec3(0.0f, 0.0f, 0.0f));
    wound = std::make_shared<Model>(".\\src\\model\\PDA_noPDA_3\\PDA_noPDA_3", 1.0f, glm::vec3(0, -2.0f, 0.0f), glm::vec3(-90.0f, 90.0f, 0.0f));
    //wound = std::make_shared<Model>(".\\src\\model\\PDA_noPDA_3\\TESTWOUND", 1.0f, glm::vec3(0, -1.0f, 0.0f), glm::vec3(0.0f, 90.0f, 0.0f));
    vessel = std::make_shared<Model>(".\\src\\model\\PDA_with_aorta\\PDA_with_aorta_no_PA_2_long5", 0.1f, glm::vec3(0, -1.5f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    clip = std::make_shared<Model>(".\\src\\model\\PDA_with_aorta\\PDA_with_aorta_no_PA_2_clip", 0.1f, glm::vec3(0, -1.5f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    vessel_squeeze = std::make_shared<Model>(".\\src\\model\\PDA_with_aorta\\PDA_with_aorta_no_PA_2_squeeze", 0.1f, glm::vec3(0, -1.5f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    leftArm = new iCROSS(LINK_PARAM_L, INI_VALUE_L, JOINT_LIMIT_L);
    rightArm = new iCROSS(LINK_PARAM_R, INI_VALUE_R, JOINT_LIMIT_R);
    leftArm->log_mode = false;
    rightArm->log_mode = false;
    printMenu();
    return true;
}

void moveWOUND() {
    static float t = 0.0f;
    t += deltaTime;
    float woundy = 0.2f * sin(t); // y 在 -0.5 ~ +0.5 之間
    if (WOUND_MOVE) {
        wound->setWorldPosition(glm::vec3(wound->getWorldPosition().x, wound->oPos.y + woundy, wound->getWorldPosition().z));
        //wound_mask->setWorldPosition(glm::vec3(wound->getWorldPosition().x, wound_mask->oPos.y + woundy, wound->getWorldPosition().z));
        if (success) {
            vessel_squeeze->setWorldPosition(glm::vec3(vessel_squeeze->getWorldPosition().x, vessel_squeeze->oPos.y + woundy, vessel_squeeze->getWorldPosition().z));
            clip->setWorldPosition(glm::vec3(clip->getWorldPosition().x, clip->oPos.y + woundy, clip->getWorldPosition().z));
        }
        if (!catched) {
            vessel->setWorldPosition(glm::vec3(vessel->getWorldPosition().x, vessel->oPos.y + woundy, vessel->getWorldPosition().z));
        }
    }
    //float woundx = 0.2f * sin(t); // y 在 -0.5 ~ +0.5 之間
    //if (WOUND_MOVE) {
    //    wound->setWorldPosition(glm::vec3(wound->oPos.x + woundx, wound->getWorldPosition().y, wound->getWorldPosition().z));
    //    //wound_mask->setWorldPosition(glm::vec3(wound->getWorldPosition().x, wound_mask->oPos.y + woundy, wound->getWorldPosition().z));
    //    if (success) {
    //        vessel_squeeze->setWorldPosition(glm::vec3(vessel_squeeze->oPos.x + woundx, vessel_squeeze->getWorldPosition().y, vessel_squeeze->getWorldPosition().z));
    //        clip->setWorldPosition(glm::vec3(clip->oPos.x + woundx, clip->getWorldPosition().y, clip->getWorldPosition().z));
    //    }
    //    if (!catched) {
    //        vessel->setWorldPosition(glm::vec3(vessel->oPos.x + woundx,vessel->getWorldPosition().y, vessel->getWorldPosition().z));
    //    }
    //}
}

void CaculateFPS() {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    fpsTimer += deltaTime;
    frameCount++;
    if (fpsTimer >= 1.0f) {
        displayFPS = frameCount / fpsTimer;
        frameCount = 0;
        fpsTimer = 0.0f;
    }
}

int CatchedCheckForcep1() {
    if (forcep1->clawClose && forcep1->clawAngle > 3.0f && forcep1->hit && !success) {
        float span1 = forcep1->clawHitpoint.y - vessel->getWorldPosition().y;
        float span2 = forcep1->clawHitpoint.z - vessel->getWorldPosition().z;
        if (span1 < 0.2f && span1 > -0.4f) {
            if (span2 < 0.2f && span2 > -0.2f && forcep1->getWorldPosition().y < -0.5f && forcep1->getWorldPosition().y > -1.1f) {
				forcep1->catched = true;
                return 1;
            }
        }
    }
	forcep1->catched = false;
    return 0;
}

void Checkcatched_stated() {
    if (forcep1->hitvessel) {
        glm::vec3 pos = forcep1->clawHitpoint;
        glm::vec3 Tpos = vessel->getWorldPosition();
        if (pos.y - Tpos.y <= -0.4) {
            uiManager->showMessage(u8"夾太深了往上一點喔", 3.0f, glm::vec3(0.5f, 1.0f, 0.5f));
        }
        else if (pos.y - Tpos.y >= 0.2) {
            uiManager->showMessage(u8"夾太淺了往下一點喔", 3.0f, glm::vec3(0.5f, 1.0f, 0.5f));
        }
        else if (pos.z - Tpos.z >= 0.2) {
            uiManager->showMessage(u8"往後一點喔", 3.0f, glm::vec3(0.5f, 1.0f, 0.5f));
        }
        else if (pos.z - Tpos.z <= -0.2) {
            uiManager->showMessage(u8"往前一點喔", 3.0f, glm::vec3(0.5f, 1.0f, 0.5f));
        }
    }
}

int CatchedCheckForcep2() {
    if (forcep2->clawClose && forcep2->clawAngle > 3.0f && forcep2->hit && !success) {
        float span1 = forcep2->clawHitpoint.y - vessel->getWorldPosition().y;
        float span2 = forcep2->clawHitpoint.z - vessel->getWorldPosition().z;
        if (span1 < 0.2f && span1 > -0.2f) {
            if (span2 < 0.1f && span2 > -0.1f) {
                return 1;
            }
        }
    }
    return 0;
}

float CheckWoundMesh() {
	float dis1, dis2, dis3, dis4;
	dis1 = WOUND_MESH[0]- HITPOINT.x;
	dis2 = HITPOINT.x - WOUND_MESH[1];
	dis3 = WOUND_MESH[2] - HITPOINT.z;
	dis4 = HITPOINT.z - WOUND_MESH[3];
	if (dis1 < 0.0f && dis2 < 0.0f && dis3 < 0.0f && dis4 < 0.0f) {
		return 1.0f;
	}
	if (dis1 < 0.0f) dis1 = 100.0f;
	if (dis2 < 0.0f) dis2 = 100.0f;
	if (dis3 < 0.0f) dis3 = 100.0f;
	if (dis4 < 0.0f) dis4 = 100.0f;
	float min1Value = glm::min(dis1, dis2);
	float min2Value = glm::min(dis3, dis4);

	float minValue = sqrt(min1Value * min1Value + min2Value * min2Value);
    if (minValue < 0.8f) {
        cout << "minValue: " << minValue << endl;
        return minValue;
    }
	cout << "out" << endl;
    return 0.0f;
}

void GameModeCheck() {
    if (CatchedCheckForcep1()) {
        uiManager->showMessage(u8"血管夾持成功！", 3.0f, glm::vec3(1.0f, 0.5f, 0.5f));
        vessel->setLocalPosition(glm::vec3(vessel->getWorldPosition().x, forcep1->clawHitpoint.y, vessel->getWorldPosition().z));
        catched = true;

        uiManager->updateMode(u8"請使用右手進行夾閉操作");
    }
    else if (forcep1->clawHitpoint.y > vessel->getWorldPosition().y) {
		catched = false;
        uiManager->updateMode(u8"請使用左手夾持血管");
    }
    
    if (CatchedCheckForcep2() && !success && forcep2->hit && catched == true) {
        success = true;
        uiManager->showMessage(u8"成功夾閉血管 手術成功!", 3.0f, glm::vec3(0.5f, 1.0f, 0.5f));
        vessel_squeeze->setLocalPosition(glm::vec3((forcep2->clawHitpointL.x + forcep2->clawHitpointU.x) / 2, vessel->getWorldPosition().y, vessel->getWorldPosition().z));
        clip->setLocalPosition(glm::vec3((forcep2->clawHitpointL.x + forcep2->clawHitpointU.x) / 2, vessel->getWorldPosition().y, vessel->getWorldPosition().z));
        uiManager->updateMode(u8"手術完成 可以按 R 重新體驗");
    }
    
    // deltaTime calculation
}

void updateController() {
    // Forcep Haptic control
    if (hapticTESTMODE && currentState == GameState::GAME) {
        forcep1->updateHaptic(deltaTime);
        forcep2->updateHaptic(deltaTime);
    }

    // Forcep Keyboard control
    if (!hapticTESTMODE && currentState == GameState::GAME) {
        forcepKeyboard(window);
        float moveSpeed = 1.0f * deltaTime;
        float rotSpeed = 45.0f * deltaTime;  // 45度/秒
        // 夾子1移動控制
        if (forcep1Control.moveForward) forcep1->moveForward(moveSpeed);
        if (forcep1Control.moveBackward) forcep1->moveForward(-moveSpeed);
        if (forcep1Control.moveLeft) forcep1->moveRight(-moveSpeed);
        if (forcep1Control.moveRight) forcep1->moveRight(moveSpeed);
        if (forcep1Control.moveUp) forcep1->moveUp(moveSpeed);
        if (forcep1Control.moveDown) forcep1->moveUp(-moveSpeed);

        // 夾子1旋轉控制
        if (forcep1Control.rotateUp) forcep1->rotate(glm::vec3(rotSpeed, 0, 0));
        if (forcep1Control.rotateDown) forcep1->rotate(glm::vec3(-rotSpeed, 0, 0));
        if (forcep1Control.rotateLeft) forcep1->rotate(glm::vec3(0, rotSpeed, 0));
        if (forcep1Control.rotateRight) forcep1->rotate(glm::vec3(0, -rotSpeed, 0));
        if (forcep1Control.rollLeft) forcep1->rotate(glm::vec3(0, 0, rotSpeed));
        if (forcep1Control.rollRight) forcep1->rotate(glm::vec3(0, 0, -rotSpeed));

        // 夾子1鉗爪控制
        if (forcep1Control.openClaw) forcep1->setClawAngle(30.0f);
        if (forcep1Control.closeClaw) forcep1->setClawAngle(0.0f);

        // 夾子2移動控制
        if (forcep2Control.moveForward) forcep2->moveForward(moveSpeed);
        if (forcep2Control.moveBackward) forcep2->moveForward(-moveSpeed);
        if (forcep2Control.moveLeft) forcep2->moveRight(-moveSpeed);
        if (forcep2Control.moveRight) forcep2->moveRight(moveSpeed);
        if (forcep2Control.moveUp) forcep2->moveUp(moveSpeed);
        if (forcep2Control.moveDown) forcep2->moveUp(-moveSpeed);

        // 夾子2旋轉控制
        if (forcep2Control.rotateUp) forcep2->rotate(glm::vec3(rotSpeed, 0, 0));
        if (forcep2Control.rotateDown) forcep2->rotate(glm::vec3(-rotSpeed, 0, 0));
        if (forcep2Control.rotateLeft) forcep2->rotate(glm::vec3(0, rotSpeed, 0));
        if (forcep2Control.rotateRight) forcep2->rotate(glm::vec3(0, -rotSpeed, 0));
        if (forcep2Control.rollLeft) forcep2->rotate(glm::vec3(0, 0, rotSpeed));
        if (forcep2Control.rollRight) forcep2->rotate(glm::vec3(0, 0, -rotSpeed));

        // 夾子2鉗爪控制
        if (forcep2Control.openClaw) forcep2->setClawAngle(30.0f);
        if (forcep2Control.closeClaw) forcep2->setClawAngle(0.0f);

        // 更新夾子動畫
        forcep1->updateClaw(deltaTime);
        forcep2->updateClaw(deltaTime);
    }
}

void UIupdate() {
    uiManager->updateFrameRate(displayFPS);
    uiManager->updateForcepPosition(1, forcep1->getWorldPosition());
    uiManager->updateForcepPosition(2, forcep2->getWorldPosition());
    uiManager->updateForcepRotation(1, forcep1->getLocalRotation());
    uiManager->updateForcepRotation(2, forcep2->getLocalRotation());
    uiManager->updateClawAngle(1, forcep1->getClawAngle());
    uiManager->updateClawAngle(2, forcep2->getClawAngle());
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void updateMesh() {
    wound->updateMesh(&collisionWorld);
    //wound_mask->updateMesh(&collisionWorld);
    if (success) {
        vessel_squeeze->updateMesh(&collisionWorld);
        clip->updateMesh(&collisionWorld);
    }
    else {
        vessel->updateMesh(&collisionWorld);
    }
    forcep1->updateMesh(&collisionWorld);
    forcep2->updateMesh(&collisionWorld);
    wound->bulletCollisionObject->setWorldTransform(btTransform(btQuaternion(0, 0, 0, 1), btVector3(wound->getWorldPosition().x, wound->getWorldPosition().y, wound->getWorldPosition().z)));
    //wound_mask->bulletCollisionObject->setWorldTransform(btTransform(btQuaternion(0, 0, 0, 1), btVector3(wound_mask->getWorldPosition().x, wound_mask->getWorldPosition().y, wound_mask->getWorldPosition().z)));
    if (success) {
        vessel_squeeze->bulletCollisionObject->setWorldTransform(btTransform(btQuaternion(0, 0, 0, 1), btVector3(vessel_squeeze->getWorldPosition().x, vessel_squeeze->getWorldPosition().y, vessel_squeeze->getWorldPosition().z)));
        clip->bulletCollisionObject->setWorldTransform(btTransform(btQuaternion(0, 0, 0, 1), btVector3(clip->getWorldPosition().x, clip->getWorldPosition().y, clip->getWorldPosition().z)));
    }
    else {
        vessel->bulletCollisionObject->setWorldTransform(btTransform(btQuaternion(0, 0, 0, 1), btVector3(vessel->getWorldPosition().x, vessel->getWorldPosition().y, vessel->getWorldPosition().z)));
    }
    forcep1->updateMesh(&collisionWorld);
    forcep2->updateMesh(&collisionWorld);
}

void updateCollision() {
    if (currentState == GameState::GAME) {
        forcep1->hit = false;
        forcep2->hit = false;
        forcep1->hitDeep = false;
        forcep2->hitDeep = false;
        forcep1->penetrate = 0;
        forcep2->penetrate = 0;
		forcep1->force = glm::vec3(0.0f, 0.0f, 0.0f);
		forcep2->force = glm::vec3(0.0f, 0.0f, 0.0f);
        forcep1->forceVec = glm::vec3(0.0f, 0.0f, 0.0f);
        forcep2->forceVec = glm::vec3(0.0f, 0.0f, 0.0f);
        forcep1->forceCount = 0;
        forcep2->forceCount = 0;
        std::vector<std::shared_ptr<Forcep>> forceps = { forcep1, forcep2 };
        checkCollisionWithWound(collisionWorld, forceps, wound.get());
        checkCollisionForceps(collisionWorld, forceps);
        if (success) {
            checkCollisionWithVessel(collisionWorld, forceps, vessel_squeeze.get());
        }
        else {
            checkCollisionWithVessel(collisionWorld, forceps, vessel.get());
        }
        forcep1->updateCollision();
        forcep2->updateCollision();

        uiManager->updateForce(1, (forcep1->getForce()));
        uiManager->updateForce(2, (forcep2->getForce()));
        //std::cout << "Forcep 1 Force: " << forcep1->getForce().x << ", " << forcep1->getForce().y << ", " << forcep1->getForce().z << std::endl;
    }
    Checkcatched_stated();
}

void printMenu() {
    if (InstructionON) {
        std::cout << "=== 雙夾子控制說明 ===" << std::endl;
        std::cout << "攝像機控制：" << std::endl;
        std::cout << "  滑鼠移動: 轉動視角" << std::endl;
        std::cout << "  滾輪: 縮放" << std::endl;
        std::cout << std::endl;
        std::cout << "夾子1控制 (左側紅色)：" << std::endl;
        std::cout << "  WASD: 前後左右移動" << std::endl;
        std::cout << "  Q/E: 上下移動" << std::endl;
        std::cout << "  I/K: 上下旋轉" << std::endl;
        std::cout << "  J/L: 左右旋轉" << std::endl;
        std::cout << "  U/O: 翻滾旋轉" << std::endl;
        std::cout << "  N/M: 開合鉗爪" << std::endl;
        std::cout << std::endl;
        std::cout << "夾子2控制 (右側藍色)：" << std::endl;
        std::cout << "  方向鍵: 前後左右移動" << std::endl;
        std::cout << "  PageUp/PageDown: 上下移動" << std::endl;
        std::cout << "  小鍵盤8/2: 上下旋轉" << std::endl;
        std::cout << "  小鍵盤4/6: 左右旋轉" << std::endl;
        std::cout << "  小鍵盤7/9: 翻滾旋轉" << std::endl;
        std::cout << "  小鍵盤+/-: 開合鉗爪" << std::endl;
        std::cout << std::endl;
        std::cout << "其他控制：" << std::endl;
        std::cout << "  R: 重置夾子位置" << std::endl;
        std::cout << "  T: 切換夾子跟隨攝像機模式" << std::endl;
        std::cout << "  ESC: 退出程序" << std::endl;
        uiManager->setInstructionsVisible(true);
    }
}

void render() {
    // 使用著色器
    ourShader->use();

    // 設定投影矩陣
    glm::mat4 projection = glm::perspective(glm::radians(camera.zoom),
        (float)ACTUAL_WIDTH / (float)ACTUAL_HEIGHT,
        0.1f, 100.0f);
    ourShader->setMat4("projection", projection);

    // 設定視圖矩陣
    glm::mat4 view = camera.getViewMatrix();
    ourShader->setMat4("view", view);

    // 設定光照
    ourShader->setVec3("lightPos", camera.position);
    ourShader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);
    ourShader->setVec3("viewPos", camera.position);

    // 渲染夾子1 (紅色)
    ourShader->setVec3("objectColor", 1.0f, 0.3f, 0.3f);
    forcep1->Draw(*ourShader);

    // 渲染夾子2 (藍色)
    ourShader->setVec3("objectColor", 0.3f, 0.3f, 1.0f);
    forcep2->Draw(*ourShader);

    ourShader->setVec3("objectColor", 0.8f, 0.3f, 0.3f);
    wound->Draw(*ourShader);
    //wound_mask->Draw(ourShader);
    if (success) {
        vessel_squeeze->Draw(*ourShader);
        clip->Draw(*ourShader);
    }
    else {
        vessel->Draw(*ourShader);
    }
    //DrawSquare(forcep1->clawHitpoint, 0.3f, glm::vec3(0, 0, 1), view, projection);
    //DrawSquare(forcep2->clawHitpoint, 0.3f, glm::vec3(0, 0, 1), view, projection);
    //DrawSquare(forcep1->forcepTail, 0.3f, glm::vec3(0, 1, 0), view, projection);
    //DrawSquare(forcep1->clawLowerPeak, 0.3f, glm::vec3(0, 0, 1), view, projection);
    //DrawSquare(forcep2->getWorldPosition(), 0.3f, glm::vec3(1, 0, 0), view, projection);
    //DrawSquare(forcep2->clawUpperPeak, 0.3f, glm::vec3(0, 1, 0), view, projection);
    //DrawSquare(forcep2->clawLowerPeak, 0.3f, glm::vec3(0, 0, 1), view, projection);
    //DrawSquare(forcep2->clawLowerPeak, 0.3f, glm::vec3(0, 0, 1), view, projection);
    
    DrawSquare(forcep1->hitpoint, 0.2f, glm::vec3(1, 0, 0), view, projection);
    DrawSquare(forcep2->hitpoint, 0.2f, glm::vec3(1, 0, 0), view, projection);
    /*if (forcep1->objU) {
        for (int i = 0; i < 20; ++i) {
            DrawSquare(forcep1->objU->arr[i], 0.2f, glm::vec3(0, 1, 0), view, projection);
        }
    }
    if (forcep1->objL) {
        for (int i = 0; i < 20; ++i) {
            DrawSquare(forcep1->objL->arr[i], 0.2f, glm::vec3(0, 0, 1), view, projection);
        }
    }
    if (forcep1->objB) {
        for (int i = 0; i < 20; ++i) {
            DrawSquare(forcep1->objB->arr[i], 0.2f, glm::vec3(1, 1, 0), view, projection);
        }
    }*/
    //DrawSquare(CHECK_UPPERPOINT, 0.3f, glm::vec3(0, 1, 0), view, projection);
    //DrawSquare(CHECK_LOWERPOINT, 0.3f, glm::vec3(0, 0, 1), view, projection);
    //DrawSquare(CHECK_BODY, 0.3f, glm::vec3(1, 1, 0), view, projection);
    //DrawSquare(ELLOPSEPOINT, 0.3f, glm::vec3(1, 0, 1), view, projection);

    DrawArrow(forcep1->hitpoint, forcep1->hitpoint + forcep1->hitdir*10.0f, glm::vec3(1, 1, 0), view, projection);
    DrawArrow(forcep2->hitpoint, forcep2->hitpoint + forcep2->hitdir*10.0f, glm::vec3(1, 1, 0), view, projection);

}

void renderUI() {
    static float blinkTime = 0.0f;
    blinkTime += deltaTime;
    if (currentState == GameState::MENU && blinkTime >= 2.0f) {
        uiManager->showMenu(u8"按下空白鍵開始進行模擬", 3.0f, glm::vec3(1.0f, 1.0f, 1.0f));
        blinkTime = 0; // 重置計時器
    }
    uiManager->updateCollisionStatus(forcep1->hit, forcep2->hit);
    uiManager->update(deltaTime);
    uiManager->render();
}

void forcepKeyboard(GLFWwindow* window) {
    // 夾子1控制 (WASD + QE + IJKL + UO + NM)
    forcep1Control.moveForward = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
    forcep1Control.moveBackward = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
    forcep1Control.moveLeft = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
    forcep1Control.moveRight = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
    forcep1Control.moveUp = glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS;
    forcep1Control.moveDown = glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS;

    forcep1Control.rotateUp = glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS;
    forcep1Control.rotateDown = glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS;
    forcep1Control.rotateLeft = glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS;
    forcep1Control.rotateRight = glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS;
    forcep1Control.rollLeft = glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS;
    forcep1Control.rollRight = glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS;

    forcep1Control.openClaw = glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS;
    forcep1Control.closeClaw = glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS;


    // 夾子2控制 (方向鍵 + PageUp/PageDown + 數字鍵)
    forcep2Control.moveForward = glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS;
    forcep2Control.moveBackward = glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS;
    forcep2Control.moveLeft = glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS;
    forcep2Control.moveRight = glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS;
    forcep2Control.moveUp = glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS;
    forcep2Control.moveDown = glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS;
    forcep2Control.rotateUp = glfwGetKey(window, GLFW_KEY_KP_8) == GLFW_PRESS;
    forcep2Control.rotateDown = glfwGetKey(window, GLFW_KEY_KP_2) == GLFW_PRESS;
    forcep2Control.rotateLeft = glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS;
    forcep2Control.rotateRight = glfwGetKey(window, GLFW_KEY_KP_6) == GLFW_PRESS;
    forcep2Control.rollLeft = glfwGetKey(window, GLFW_KEY_KP_7) == GLFW_PRESS;
    forcep2Control.rollRight = glfwGetKey(window, GLFW_KEY_KP_9) == GLFW_PRESS;

    forcep2Control.openClaw = glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS;
    forcep2Control.closeClaw = glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS;
}

void processInput(GLFWwindow* window) {

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
		currentState = GameState::EXIT; // 設置遊戲狀態為退出
    }
	// 切換遊戲狀態

    if (currentState == GameState::GAME) {
        // F1 鍵切換說明顯示
        if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS && !f1Pressed) {
            uiManager->toggleInstructions();
            f1Pressed = true;
        }
        if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_RELEASE) {
            f1Pressed = false;
        }

        // 切換跟隨模式
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && !tPressed) {
            uiManager->showMessage(u8"切換模式", 2.0f, glm::vec3(0.5f, 1.0f, 0.5f));
            tPressed = true;
        }
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE) {
            tPressed = false;
        }

        // 重置夾子位置
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !rPressed) {
            // 重置邏輯可以在這裡添加
            uiManager->showMessage(u8"已重製體驗流程 ", 2.0f, glm::vec3(1.0f, 0.5f, 0.5f));
            rPressed = true;
        }
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) {
            rPressed = false;
        }


	}
	else if (currentState == GameState::MENU) {
		// 空格鍵開始遊戲
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			currentState = GameState::GAME;
            uiManager->updateMode(u8"請使用左手夾持血管");
			//uiManager->hideMenu();
			uiManager->showMessage(u8"手術模擬開始", 2.0f, glm::vec3(0.5f, 1.0f, 0.5f));
			InstructionON = false;
			success = false; // 重置成功狀態
		}
	}
    // Reset forcep
    if (rPressed) {
        forcep1->reset();
        forcep2->reset();
        success = false;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    if (uiManager) {
        uiManager->setScreenSize(width, height);
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (currentState != GameState::GAME) return; 
        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }


        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos;

        lastX = xpos;
        lastY = ypos;

        camera.processMouseMovement(xoffset, yoffset);
    
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	if (currentState != GameState::GAME) return; 
    camera.zoom -= (float)yoffset;
    if (camera.zoom < 1.0f)
        camera.zoom = 1.0f;
    if (camera.zoom > 45.0f)
        camera.zoom = 45.0f;
}

float distanceToEllipseXZ(const glm::vec3& point, float a, float b, float cx, float cz) {
    float x = point.x - cx;
    float z = point.z - cz;
    float norm = std::sqrt((x * x) / (a * a) + (z * z) / (b * b));
    if (norm == 0.0f) return a; // 圓心特例
    // 橢圓邊界上的最近點
    
    float ex = a * x / (norm * a);
    float ez = b * z / (norm * b);
	ELLOPSEPOINT = glm::vec3(ex + cx, point.y, ez + cz);
    float dist = std::sqrt((x - ex) * (x - ex) + (z - ez) * (z - ez));
    if (norm < 1.0f) dist = -dist;
    return dist;
}

float distanceToEllipsoidSurface(const glm::vec3& point, const glm::vec3& center, float a, float b, float c) {
    // 先將點轉換到橢球中心座標系
    glm::vec3 p = point - center;
    // 計算歸一化向量
    float norm = std::sqrt(
        (p.x * p.x) / (a * a) +
        (p.y * p.y) / (b * b) +
        (p.z * p.z) / (c * c)
    );
    // 橢球表面上的最近點
    glm::vec3 surfacePoint = glm::vec3(
        a * p.x / (norm * a),
        b * p.y / (norm * b),
        c * p.z / (norm * c)
    );

    //ELLOPSEPOINT = surfacePoint;
    // 轉回世界座標
    surfacePoint += center;
    // 距離
    float dist = glm::distance(point, surfacePoint);
    // 如果點在橢球內部，回傳負值
    if (norm < 1.0f) dist = -dist;
    return dist;
}

void checkCollisionWithWound_0818(btCollisionWorld& world, const std::vector<std::shared_ptr<Forcep>>& forceps, Model* wound) {
      if(currentState != GameState::GAME) return;
      for (auto& f : forceps) {
          if (!f->objU || !f->objL || f->hit) continue;
          std::vector<btCapsuleOBJ*> objs = { f->objU, f->objL, f->objB };
          for (auto& obj : objs) {
              //MyResult2Callback result(obj->Trans);
              SurfaceNormalCallback result(obj->Trans);
              world.contactPairTest(&(obj->Object), wound->bulletCollisionObject, result);
              if (result.hit && result.penetrationDepth > f->penetrate) {
                  f->penetrate = result.penetrationDepth;
                  f->hit = true;
                  glm::vec3 dir = f->prePos - f->getWorldPosition();
				  //glm::vec3 dir = obj->A - obj->B;
                  f->hitpoint = glm::vec3(result.hitPoint.x(), result.hitPoint.y(), result.hitPoint.z());
                  HITPOINT = glm::vec3(result.hitPoint.x(), result.hitPoint.y(), result.hitPoint.z());
                  //NORMAL_DIR = dir * 100.0f;
                  //if (!f->prehit) {
				  if (result.penetrationDepth < 0.004f) {
                        f->hitdir = glm::vec3(result.surfaceNormal.x(), result.surfaceNormal.y(), result.surfaceNormal.z());
                  }
				  if (result.penetrationDepth > 0.004f) {
					  f->setRelatedWorldPosition(dir * 0.02f);
				  }
                  NORMAL_DIR = f->hitdir;
                  float dot = glm::dot(-dir, f->hitdir);
				  /*std::cout << "dir: " << dir.x << ", " << dir.y << ", " << dir.z << std::endl;
				  std::cout << "NORMAL_DIR: " << NORMAL_DIR.x << ", " << NORMAL_DIR.y << ", " << NORMAL_DIR.z << std::endl;
                  */
                  float dirALL = sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
                  float rate1 = dirALL / 0.02f;
                  float rate2 = 0.5f / dirALL;
                  float rate = 0.0f;
                  if (dirALL == 0)
                      rate1 = 0.0f; // 避免除以零
                  rate = rate1 + rate2;/*
				  std::cout << "rate1" << rate1 << std::endl;
				  std::cout << "rate2" << rate2 << std::endl;*/
                  



				  glm::vec3 force(f->hitdir.x, f->hitdir.y, f->hitdir.z);
				  float value = (result.penetrationDepth)/0.02 + 0.5f;
                  if (result.penetrationDepth > 0.04) {
                      value = 3.0f;
                  }
                  /*float value = (result.penetrationDepth)/0.002 + 0.5f;
                  if (value > 2.8f) {
					  value = 2.8f;
                  }*/
                  /*if (result.penetrationDepth > 0.003 ) {
                      value = rate;
                  }*/
				  std::cout << "Penetration Depth: " << result.penetrationDepth << std::endl;
                  if (dot <= 0) {
                      f->force = force * value;
                  }
				  std::cout << "value" << value << std::endl;
				  std::cout << "Force: " << f->force.x << ", " << f->force.y << ", " << f->force.z << std::endl;
                  f->prePos += (wound->getWorldPosition() - wound->prePos);
                  glm::vec3 pushDir = wound->getWorldPosition() - wound->prePos;
                  glm::vec3 forcepToWound = f->getWorldPosition() - wound->prePos;
                  if (glm::dot(pushDir, forcepToWound) > 0) {
                      f->setRelatedWorldPosition(pushDir);
                  }
              }
          }
          
          f->prehit = f->hit;
      }
      wound->prePos = wound->getWorldPosition();
  }

void checkCollisionWithWound_0825(btCollisionWorld& world, const std::vector<std::shared_ptr<Forcep>>& forceps, Model* wound) {
     if (currentState != GameState::GAME) return;
     for (auto& f : forceps) {
         if (!f->objU || !f->objL || f->hit) continue;
         std::vector<btCapsuleOBJ*> objs = { f->objU, f->objL, f->objB };
         for (auto& obj : objs) {
             //MyResult2Callback result(obj->Trans);
             SurfaceNormalCallback result(obj->Trans);
             world.contactPairTest(&(obj->Object), wound->bulletCollisionObject, result);
             if (result.hit) {
                 f->hit = true;
                 //std::cout << "UPOS: " << f->getWorldPosition().x << ", " << f->getWorldPosition().y << ", " << f->getWorldPosition().z << std::endl;
                 //std::cout << "prePOS: " << f->prePos.x << ", " << f->prePos.y << ", " << f->prePos.z << std::endl;
                 glm::vec3 dir = f->prePos - f->getWorldPosition();
                 HITPOINT = glm::vec3(result.hitPoint.x(), result.hitPoint.y(), result.hitPoint.z());
                 //NORMAL_DIR = dir * 100.0f;
                 NORMAL_DIR = glm::vec3(result.surfaceNormal.x(), result.surfaceNormal.y(), result.surfaceNormal.z());

                 std::cout << "dir: " << dir.x << ", " << dir.y << ", " << dir.z << std::endl;
                 std::cout << "NORMAL_DIR: " << NORMAL_DIR.x << ", " << NORMAL_DIR.y << ", " << NORMAL_DIR.z << std::endl;
                 glm::vec3 hitP = glm::vec3(result.hitPoint.x(), result.hitPoint.y(), result.hitPoint.z());
                 //glm::vec3 dir = obj->C - hitP;
                 //glm::vec3 dir = f->clawHitpointU - f->getWorldPosition();
                 //std::cout << "hit Dir: " << dir.x << ", " << dir.y << ", " << dir.z << std::endl;
                 float dirALL = sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
                 float rate1 = 0.0f;
                 float rate2 = 0.0f;
                 float rate = 0.0f;


                 if (dirALL > 0.001f) {
                     f->hitDeep = true;
                 }
                 else {
                     f->hitDeep = false;
                 }

                 if (!f->hitDeep) {
                     glm::vec3 force(f->speed.x, f->speed.y, f->speed.z);
                     f->force = force * (-1.0f); // 設定回饋力
                     f->prePos += f->speed;
                 }
                 else {
                     rate1 = dirALL / 0.02f;
                     rate2 = 0.5f / dirALL;
                     rate = rate1 + rate2;
                     glm::vec3 force(dir.x * rate, dir.y * rate, dir.z * rate);
                     f->force = force * 1.0f;
                 }
                 /*********************************************/
                 /*if (result.penetrationDepth > 0.02f) {
                     f->setRelatedWorldPosition(dir*0.1f);
                 }*/



                 //std::cout << "Force: " << f->force << std::endl;



                 //        else if (dirALL < 0.02f) {
                             //rate1 = 0.0f; // 當距離小於 0.02 時，回饋力為 0
                 //        }

                 //std::cout << "dirALL: " << dirALL << std::endl;
                 //std::cout << "rate: " << rate * dirALL << std::endl;

                 f->prePos += (wound->getWorldPosition() - wound->prePos);
                 glm::vec3 pushDir = wound->getWorldPosition() - wound->prePos;
                 glm::vec3 forcepToWound = f->getWorldPosition() - wound->prePos;
                 if (glm::dot(pushDir, forcepToWound) > 0) {
                     f->setRelatedWorldPosition(pushDir);
                 }
             }
         }
     }
     wound->prePos = wound->getWorldPosition();
 }

void checkCollisionWithWound_0901(btCollisionWorld& world, const std::vector<std::shared_ptr<Forcep>>& forceps, Model* wound) {
    if (currentState != GameState::GAME) return;
    for (auto& f : forceps) {
        if (!f->objU || !f->objL) continue;
        std::vector<btCapsuleOBJ*> objs = { f->objU, f->objL, f->objB };
        for (auto& obj : objs) {
            SurfaceNormalCallback result(obj->Trans);
            world.contactPairTest(&(obj->Object), wound->bulletCollisionObject, result);
            if (result.hit) {
                f->hit = true;
                glm::vec3 dir;
				float forceRate = 1.0f;
				HITPOINT = result.hitPointGLM;
                NORMAL_DIR = glm::vec3(result.surfaceNormal.x(), result.surfaceNormal.y(), result.surfaceNormal.z());

                float dist = distanceToEllipseXZ(HITPOINT,1.06, 1.67f, wound->getWorldPosition().x+0.01f, wound->getWorldPosition().z+0.375f);
				float dist3D = distanceToEllipsoidSurface(HITPOINT, wound->getWorldPosition() + glm::vec3(0.01f, 2.0f, 0.375f), 1.06f, 2.43f, 1.67f);
                if (dist <= -0.2f) {
                    
                    dir = glm::normalize(wound->getWorldPosition() + glm::vec3(0.0f, 1.0f, 0.0f) - HITPOINT);
					f->OutOfWound = false;
                }
                else if (dist >= 0.4f) {
                    dir = glm::vec3(0.0f, 1.0f, 0.0f);
                    float dirALL = abs(f->prePosFORWOUND.y - f->getWorldPosition().y);
                    float rate1 = 0.0f;
                    float rate2 = 0.0f;
                    rate1 = dirALL / 0.02f;
                    rate2 = 0.5f / dirALL;
                    if (dirALL == 0)
                        rate2 = 0.0f;
                    forceRate = (rate1 + rate2)* dirALL;
					f->OutOfWound = true;
				}
				else {
					//glm::vec3 dir1 = (dist * glm::vec3(0.0f, abs(f->prePosFORWOUND.y - f->getWorldPosition().y), 0.0f) + (1.0f - dist) * glm::normalize(wound->getWorldPosition() + glm::vec3(0.0f, 1.0f, 0.0f) - HITPOINT)) / 1.0f;
					glm::vec3 dir2 = glm::normalize(wound->getWorldPosition() + glm::vec3(0.0f, 1.0f, 0.0f) - HITPOINT);
                    /*float dirALL = sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
					float rate1 = 0.0f;
					float rate2 = 0.0f;
					float rate = 0.0f;
					rate1 = dirALL / 0.02f;
					rate2 = 0.5f / dirALL;
					if (dirALL == 0)
						rate1 = 0.0f;
					rate = rate1 + rate2;
					dir1 = rate1 * dir1;
					dir = (dist + 0.8f) * dir1 + (1.0f - (dist+0.8f)/1.2f) * dir2;*/
                    glm::vec3 dir1 = glm::vec3(0.0f, 1.0f, 0.0f);
                    float dirALL = abs(f->prePosFORWOUND.y - f->getWorldPosition().y);
                    float rate1 = 0.0f;
                    float rate2 = 0.0f;
                    rate1 = dirALL / 0.02f;
                    rate2 = 0.5f / dirALL;
                    if (dirALL == 0)
                        rate2 = 0.0f;
                    forceRate = (rate1 + rate2) * dirALL;

                    dir = (dist + 0.2f) * dir1 + (1.0f - (dist + 0.2f) / 0.6f) * dir2;

                    f->OutOfWound = true;
                }
                
				f->hitpoint = HITPOINT;
                //std::cout << "HITPOINT ( " << HITPOINT.x << " , " << HITPOINT.y << " , " << HITPOINT.z << " ) " << std::endl;
				//f->hitdir = dir * forceRate;
				std::cout << "dist3D: " << dist3D << std::endl;
                f->forceVec += dir * forceRate;
                f->forceCount++;

                f->prePosFORWOUND += (wound->getWorldPosition() - wound->prePos);
                f->prePos += (wound->getWorldPosition() - wound->prePos);
                glm::vec3 pushDir = wound->getWorldPosition() - wound->prePos;
                glm::vec3 forcepToWound = f->getWorldPosition() - wound->prePos;
                if (glm::dot(pushDir, forcepToWound) > 0) {
                    f->setRelatedWorldPosition(pushDir);
                }

            }
        }
    }
    wound->prePos = wound->getWorldPosition();
}

void checkCollisionWithWound_0828(btCollisionWorld& world, const std::vector<std::shared_ptr<Forcep>>& forceps, Model* wound) {
    if (currentState != GameState::GAME) return;
    for (auto& f : forceps) {
        if (!f->objU || !f->objL || f->hit) continue;
        std::vector<btCapsuleOBJ*> objs = { f->objU, f->objL, f->objB };
        for (auto& obj : objs) {
            SurfaceNormalCallback result(obj->Trans);
            world.contactPairTest(&(obj->Object), wound->bulletCollisionObject, result);
            if (result.hit && !f->hit) {
                f->hit = true;
                glm::vec3 dir;
                std::cout << "Penetrate:" << result.penetrationDepth << std::endl;
                HITPOINT = result.hitPointGLM;
                NORMAL_DIR = glm::vec3(result.surfaceNormal.x(), result.surfaceNormal.y(), result.surfaceNormal.z());
                dir = f->prePos - f->getWorldPosition();
                float dirALL = sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
                float rate1 = 0.0f;
                float rate2 = 0.0f;
                float rate = 0.0f;

                rate1 = dirALL / 0.02f;
                rate2 = 0.5f / dirALL;
                if (dirALL == 0)
                    rate1 = 0.0f;
                rate = rate1 + rate2;

                glm::vec3 force;
                glm::vec3 dir1 = glm::vec3(0.0f, dir.y, 0.0f);
                glm::vec3 dir2 = dirALL * glm::normalize(wound->getWorldPosition() + glm::vec3(0.0f, 1.0f, 0.0f) - HITPOINT);
                if (CheckWoundMesh() == 1.0f) {
                    force = glm::vec3(dir2.x * rate, dir2.y * rate, dir2.z * rate);
                }
                else if (CheckWoundMesh() == 0.0f) {
                    force = glm::vec3(0.3f, dir1.y * rate, 0.3f);
                    //std::cout << "Only Y force applied" << std::endl;
                }
                else {
                    float dis = CheckWoundMesh();
                    glm::vec3 dir3 = (dir2 * (0.8f - dis) + dir1 * dis) / 0.8f;
                    force = glm::vec3(dir3.x * rate, dir3.y * rate, dir3.z * rate);
                }
                f->hitpoint = HITPOINT;
                f->hitdir = force * 4.0f;
                f->force = force * 1.0f;


                f->prePos += (wound->getWorldPosition() - wound->prePos);
                glm::vec3 pushDir = wound->getWorldPosition() - wound->prePos;
                glm::vec3 forcepToWound = f->getWorldPosition() - wound->prePos;
                if (glm::dot(pushDir, forcepToWound) > 0) {
                    f->setRelatedWorldPosition(pushDir);
                }

            }
        }
    }
    wound->prePos = wound->getWorldPosition();
}

void checkCollisionWithWound_(btCollisionWorld& world, const std::vector<std::shared_ptr<Forcep>>& forceps, Model* wound) {
    if (currentState != GameState::GAME) return;
    for (auto& f : forceps) {
        if (!f->objU || !f->objL || f->hit) continue;
        std::vector<btCapsuleOBJ*> objs = { f->objU, f->objL, f->objB };
		btCapsuleOBJ* obj = nullptr;
        for (auto& obj : objs) {
            //MyResult2Callback result(obj->Trans);
            SurfaceNormalCallback result(obj->Trans);
            world.contactPairTest(&(obj->Object), wound->bulletCollisionObject, result);
            if (result.hit) {
                f->hit = true;
                //std::cout << "UPOS: " << f->getWorldPosition().x << ", " << f->getWorldPosition().y << ", " << f->getWorldPosition().z << std::endl;
                //std::cout << "prePOS: " << f->prePos.x << ", " << f->prePos.y << ", " << f->prePos.z << std::endl;
				obj->hit = true;
                /*for (auto& point : obj->arr) {
					float depth = glm::distance(point, result.hitPointGLM);
                    if (depth < obj->hitdepth || obj->hitdepth == -1.0f) {
						obj->hitdepth = depth;
						obj->hitpoint = point;
						obj->hitdir = result.hitPointGLM - point;
                    }
                }*/
                glm::vec3 point = obj->C;
                float depth = glm::distance(point, result.hitPointGLM);
                obj->hitdepth = depth;
                obj->hitpoint = point;
                obj->hitdir = point - result.hitPointGLM;
				f->hitdir = obj->hitdir*4.0f;
				f->hitpoint = result.hitPointGLM;
                
                
                
                
                glm::vec3 dir = f->hitdir;
                float dirALL = sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
                float rate1 = 0.0f;
                float rate2 = 0.0f;
                float rate = 0.0f;


                if (dirALL > 0.001f) {
                    f->hitDeep = true;
                }
                else {
                    f->hitDeep = false;
                }

                if (!f->hitDeep) {
                    glm::vec3 force(f->speed.x, f->speed.y, f->speed.z);
                    //f->force = force * (-1.0f); // 設定回饋力
                    f->prePos += f->speed;
                }
                else {
                    rate1 = dirALL / 0.02f;
                    rate2 = 0.5f / dirALL;
                    rate = rate1 + rate2;
                    glm::vec3 force(dir.x * rate, dir.y * rate, dir.z * rate);
                    //f->force = force * 1.0f;
                }
                /*if (result.penetrationDepth > 0.02f) {
                    f->setRelatedWorldPosition(dir * 0.1f);
                }*/

                f->prePos += (wound->getWorldPosition() - wound->prePos);
                glm::vec3 pushDir = wound->getWorldPosition() - wound->prePos;
                glm::vec3 forcepToWound = f->getWorldPosition() - wound->prePos;
                if (glm::dot(pushDir, forcepToWound) > 0) {
                    f->setRelatedWorldPosition(pushDir);
                }
            }
        }
    }
    wound->prePos = wound->getWorldPosition();
}

void checkCollisionWithWound(btCollisionWorld& world, const std::vector<std::shared_ptr<Forcep>>& forceps, Model* wound) {
    if (currentState != GameState::GAME) return;
    for (auto& f : forceps) {
        if (!f->objU || !f->objL) continue;
        std::vector<btCapsuleOBJ*> objs = { f->objU, f->objL, f->objB };
        bool testHit = false;
        for (auto& obj : objs) {
            SurfaceNormalCallback result(obj->Trans);
            world.contactPairTest(&(obj->Object), wound->bulletCollisionObject, result);
            if (result.hit) {
				testHit = true;
                f->hit = true;
                glm::vec3 dir;
                float forceRate = 1.0f;
                HITPOINT = result.hitPointGLM;
                NORMAL_DIR = glm::vec3(result.surfaceNormal.x(), result.surfaceNormal.y(), result.surfaceNormal.z());

                float dist = distanceToEllipseXZ(HITPOINT, 1.06, 1.9f, wound->getWorldPosition().x + 0.01f, wound->getWorldPosition().z + 0.5f);
                float dist3D = distanceToEllipsoidSurface(HITPOINT, wound->getWorldPosition() + glm::vec3(0.01f, 2.0f, 0.375f), 1.06f, 2.43f, 1.67f);
                
                if (dist >= 0.1f && f->mode != 3 && f->mode != 2) {
                    dir = glm::vec3(0.0f, 1.0f, 0.0f);
                    float dirALL = abs(f->prePosFORWOUND.y - f->getWorldPosition().y);
					//std::cout << "dirALL" << dirALL << std::endl;
                    float rate1 = 0.0f;
                    float rate2 = 0.0f;
                    rate1 = dirALL / 0.02f;
                    rate2 = 0.5f / dirALL;
                    if (dirALL == 0)
                        rate2 = 0.0f;
                    forceRate = (rate1 + rate2) * dirALL;
                    f->OutOfWound = true;
                    f->mode = 1;
                }
                else if ((HITPOINT.y > wound->getWorldPosition().y && (f->mode == 0 || f->mode == 3 || f->mode == 2))) {
					//std::cout << "distance" << dist << std::endl;
                    dir = glm::normalize(wound->getWorldPosition() + glm::vec3(0.0f, 1.0f, 0.0f) - HITPOINT);
                    float rate1 = 0.0f;
                    float rate2 = 0.0f;
                    dist = abs(dist);
					std::cout << "dist" << dist << std::endl;
                    rate1 = dist / 0.08f;
                    rate2 = 0.5f / dist;
                    if (dist == 0)
                        rate2 = 0.0f;
                    forceRate = (rate1 + rate2) * dist;
                    if (forceRate > 1.2f) {
						forceRate = 1.2f;
                    }

                    forceRate = 0.6f;
                    f->OutOfWound = false;
                    f->mode = 3;
                }
                else {
                    dir = glm::normalize(wound->getWorldPosition() + glm::vec3(0.0f, 1.0f, 0.0f) - HITPOINT);
                    f->OutOfWound = false;
					f->mode = 2;
                    /*if (f->hitvessel) {
						forceRate = 0.2f;
                    }*/
                    
                }
                if (dist >= 0.1f && f->getWorldPosition().y <= -0.8) {
                    uiManager->showMessage(u8"你不乖喔 這樣很痛", 2.0f, COLOR_ERROR);
                }
				//std::cout << "woundPOS" << wound->getWorldPosition().y << std::endl;
				//std::cout << "HITPOINT" << HITPOINT.y << std::endl;
                f->hitpoint = HITPOINT;
                //std::cout << "HITPOINT ( " << HITPOINT.x << " , " << HITPOINT.y << " , " << HITPOINT.z << " ) " << std::endl;
                //f->hitdir = dir * forceRate;
                //std::cout << "dist3D: " << dist3D << std::endl;
			    std:;cout << "forcerate: " << forceRate << std::endl;
                
                f->forceVec += dir * forceRate;
                f->forceCount++;

                f->prePosFORWOUND += (wound->getWorldPosition() - wound->prePos);
                f->prePos += (wound->getWorldPosition() - wound->prePos);
                glm::vec3 pushDir = wound->getWorldPosition() - wound->prePos;
                glm::vec3 forcepToWound = f->getWorldPosition() - wound->prePos;
                if (glm::dot(pushDir, forcepToWound) > 0) {
                    f->setRelatedWorldPosition(pushDir);
                }

            }
        }
        if (!testHit) {
			f->mode = 0;    
        }
		//std::cout << "mode: " << f->mode << std::endl;  
    }
    wound->prePos = wound->getWorldPosition();
}

void checkCollisionWithVessel(btCollisionWorld& world, const std::vector<std::shared_ptr<Forcep>>& forceps, Model* vessel) {
    if(currentState != GameState::GAME) return;
    for (auto& f : forceps) {
        if (!f->objU || !f->objL)  continue;
        std::vector<btCapsuleOBJ*> objs = { f->objU, f->objL , f->objB };
        bool hitvessel = false;
        for (auto& obj : objs) {
            MyResultCallback result(obj->Trans);
            world.contactPairTest(&(obj->Object), vessel->bulletCollisionObject, result);
            if (result.hit) {
				hitvessel = true;
				HITPOINT = glm::vec3(result.hitPoint.x(), result.hitPoint.y(), result.hitPoint.z());
				CHECK_UPPERPOINT = f->clawUpperPeak;
				CHECK_LOWERPOINT = f->clawLowerPeak;
				CHECK_BODY = f->getWorldPosition();
				float distance = glm::distance(CHECK_UPPERPOINT, CHECK_BODY);

                float dist = distanceToEllipseXZ(HITPOINT, 1.06, 1.67f, wound->getWorldPosition().x + 0.01f, wound->getWorldPosition().z + 0.375f);
				if (dist >= 0.1f) {
					f->OutOfWound = true;
				}
				else {
					f->OutOfWound = false;
				}

				//std::cout << "Distance: " << distance << std::endl;
				//std::cout << "Hit Point: " << HITPOINT.x << ", " << HITPOINT.y << ", " << HITPOINT.z << std::endl;
				//std::cout << "POS: " << CHECK_UPPERPOINT.x << ", " << CHECK_UPPERPOINT.y << ", " << CHECK_UPPERPOINT.z << std::endl;
                f->hit = true;
                glm::vec3 dir = f->prePos - f->getWorldPosition();
				NORMAL_DIR = dir*100.0f;
                //std::cout << "hit Dir: " << dir.x << ", " << dir.y << ", " << dir.z << std::endl;
                float dirALL = sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
                float rate1 = dirALL / 0.02f;
                float rate2 = 0.5f / dirALL;
                float rate = 0.0f;
                if (dirALL == 0)
                    rate1 = 0.0f; // 避免除以零
                rate = rate1 + rate2;
                float dirx = 0.3f;
				if (dir.x < 0)
					dirx = -0.3f;
                glm::vec3 force(0, dir.y * rate, dir.z * rate);
                /*if (f->num == 2) {
                    force = glm::vec3(0, 0, dir.z * rate);
                }*/
                if (!f->OutOfWound) {
                    f->forceVec += force * 1.0f; // 設定回饋力
                    f->forceCount++;
                }
            }
        }
		
		f->hitvessel = hitvessel;
    }
    vessel->prePos = vessel->getWorldPosition();
}

void checkCollisionForceps(btCollisionWorld& world, const std::vector<std::shared_ptr<Forcep>>& forceps) {
    if (currentState != GameState::GAME) return;
    auto f1 = forceps[0];
    auto f2 = forceps[1];
    //printf("test");
    if (!f1->objU || !f1->objL|| !f2->objU || !f2->objL) return;
    std::vector<btCapsuleOBJ*> objs1 = { f1->objU, f1->objL , f1->objB };
    std::vector<btCapsuleOBJ*> objs2 = { f2->objU, f2->objL , f2->objB };
    for (auto& obj : objs1) {
        MyResultCallback result(obj->Trans);
        for (auto& obj2 : objs2) {
            // 檢查兩個夾子之間的碰撞
            world.contactPairTest(&(obj->Object), &(obj2->Object), result);
            if (result.hit) {
                //printf("test1");
                //HITPOSITION = glm::vec3(result.hitPoint.x(), result.hitPoint.y(), result.hitPoint.z());
                f1->hit = true;
                f2->hit = true;
                glm::vec3 dir1 = f1->prePos - f1->getWorldPosition();
                glm::vec3 dir2 = f2->prePos - f2->getWorldPosition();
                //std::cout << "hit Dir1: " << dir1.x << ", " << dir1.y << ", " << dir1.z << std::endl;
                //std::cout << "hit Dir2: " << dir2.x << ", " << dir2.y << ", " << dir2.z << std::endl;
                float dirALL1 = sqrt(dir1.x * dir1.x + dir1.y * dir1.y + dir1.z * dir1.z);
                float dirALL2 = sqrt(dir2.x * dir2.x + dir2.y * dir2.y + dir2.z * dir2.z);
                float rate11 = dirALL1 / 0.01f;
                float rate21 = dirALL2 / 0.01f;
                float rate12 = 0.8f / dirALL1;
                float rate22 = 0.8f / dirALL2;
                if (dirALL1 == 0)
                    rate12 = 0.0f;
                if (dirALL2 == 0)
                    rate22 = 0.0f;
                float rateALL1 = rate11 + rate12;
                float rateALL2 = rate21 + rate22;
                glm::vec3 force1(dir1.x * rateALL1, dir1.y * rateALL1, dir1.z * rateALL1);
                glm::vec3 force2(dir2.x * rateALL2, dir2.y * rateALL2, dir2.z * rateALL2);
                f1->forceVec += force1 * 1.0f;
                f2->forceVec += force2 * 1.0f;
                f1->forceCount++;
                f2->forceCount++;
                break;
            }
        }
    }
}

void udpLoop() {
    while (true) {
        if (currentState == GameState::GAME) {
            std::string robotCommand = generateRobotCommand(forcep1, forcep2);
            memcpy(pServer->Send_Message, robotCommand.c_str(), std::min(robotCommand.length(), (size_t)BUFFER_LEN));
            
            static int debugCounter = 0;
            if (debugCounter % 100 == 0) { 
                //std::cout << "Robot Command: " << robotCommand << std::endl;
            }
            debugCounter++;
            pServer->send_data(TARGET_IP, TARGET_PORT);
        }
        if (currentState == GameState::EXIT) {
            break;
        }
        
        // 控制發送頻率
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // 100Hz
    }
    return;
}

hduMatrix getForcepWorldMatrix(glm::vec3 pos, glm::mat3 R) {
    hduMatrix targetPose;
    // 設定位置
    targetPose[0][3] = pos.x;
    targetPose[1][3] = pos.y;
    targetPose[2][3] = pos.z;
    // 設定旋轉矩陣
    targetPose[0][0] = R[0][0]; targetPose[0][1] = R[0][1]; targetPose[0][2] = R[0][2];
    targetPose[1][0] = R[1][0]; targetPose[1][1] = R[1][1]; targetPose[1][2] = R[1][2];
    targetPose[2][0] = R[2][0]; targetPose[2][1] = R[2][1]; targetPose[2][2] = R[2][2];
	return targetPose;
}

std::string generateRobotCommand(const std::shared_ptr<Forcep>& forcep1,
    const std::shared_ptr<Forcep>& forcep2) {
	static std::string RobotDefaultCommand = "0,0,-90,-90,-60,-360,350,0,0,-90,-60,-360,0,0,0,0";
	static Eigen::MatrixXd lastLeftJoints(6, 1);
	static Eigen::MatrixXd lastRightJoints(6, 1);
	/*Eigen::MatrixXd leftJoints(6, 1);
	Eigen::MatrixXd rightJoints(6, 1);*/
    glm::vec3 pos1 = forcep1->getUDPPosition();
    glm::vec3 pos2 = forcep2->getUDPPosition();
    if (!leftArm || !rightArm) {
        return RobotDefaultCommand;
    }
    glm::mat3 R1 = { 0.939693,-0.000000,-0.342020,0.000000,1.000000,-0.000000,0.342020,0.000000,0.939693};
	glm::mat3 R2 = { 0.939693,0.000000,0.342020,0.000000,1.000000,0.000000,-0.342020,0.000000,0.939693 };
   
    hduMatrix leftMatrix = getForcepWorldMatrix(pos1, R1);
    hduMatrix rightMatrix = getForcepWorldMatrix(pos2, R2);

    leftArm->update_targetPosition(leftMatrix);
    rightArm->update_targetPosition(rightMatrix);

    bool leftSolved = leftArm->solve_inverse();
    bool rightSolved = rightArm->solve_inverse();
	/*if (!leftSolved || !rightSolved) {
		std::cerr << "Inverse kinematics solution failed!" << std::endl;
		return lastRobotCommand;
    }*/
	auto leftJoints = leftArm->current_value;
	auto rightJoints = rightArm->current_value;
    if (leftSolved) {
		lastLeftJoints = leftArm->current_value;
        leftJoints = leftArm->current_value;
    }
    else {
		leftJoints = lastLeftJoints;
    }
	if (rightSolved) {
		lastRightJoints = rightArm->current_value;
        rightJoints = rightArm->current_value;
    }
    else {
		rightJoints = lastRightJoints;
    }


    char jointMessage[BUFFER_LEN] = { "0" };
    // Step 5: 組成 UDP 字串
    bool bt11 = false;
	bool bt12 = false;
	bool bt21 = false;
	bool bt22 = false;
    unsigned char leftBtn[2] = {
        bt11,           // L0: toggle
        (unsigned char)bt12 // L1: instant
    };
    unsigned char rightBtn[2] = {
        bt21,           // R0: toggle
        (unsigned char)bt22 // R1: instant
    };
    // char jointMessage[BUFFER_LEN] = { 0 };
    sprintf(jointMessage,
        "%.0f,%.0f,%.0f,%.0f,%.0f,%.0f,%.0f,%.0f,%.0f,%.0f,%.0f,%.0f,%u,%u,%u,%u",
        leftJoints[0], leftJoints[1],
        leftJoints[2] * 180 / M_PI, leftJoints[3] * 180 / M_PI,
        leftJoints[4] * 180 / M_PI, leftJoints[5] * 180 / M_PI,
        rightJoints[0], rightJoints[1],
        rightJoints[2] * 180 / M_PI, rightJoints[3] * 180 / M_PI,
        rightJoints[4] * 180 / M_PI, rightJoints[5] * 180 / M_PI,
        leftBtn[0], leftBtn[1], rightBtn[0], rightBtn[1]
    );
    return jointMessage;
}