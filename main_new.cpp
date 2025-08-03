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

// my lib
#include "shader.h"
#include "Model_new.h"
#include "Forcep_new.h"
#include "Tool.h"
#include "HapticDevice.h"
#include "UIManager.h"
#include "Constant.h"
#include "GameEngine.h"


// Window setup
const unsigned int SCR_WIDTH = WIDTH;
const unsigned int SCR_HEIGHT = HEIGHT;

// 全域螢幕尺寸變量
unsigned int ACTUAL_WIDTH = SCR_WIDTH;
unsigned int ACTUAL_HEIGHT = SCR_HEIGHT;

GameState currentState = GameState::MENU; // 初始狀態為主菜單

// Camera setup
Camera camera(CAMERA_POSITION);
// Variables
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

bool mouseCONTROL = MOUSE_INVISIBLE; // 是否啟用滑鼠控制攝像機
bool InstructionON = SHOW_INSTRUCTION; // 是否顯示控制說明
bool hapticTESTMODE = HAPTIC_MODE; // 是否啟用觸覺測試模式

static bool tPressed = false;
static bool rPressed = false;
static bool f1Pressed = false;
static bool success = false;


float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool firstMouse = true;

ForcepControl forcep1Control;  // WASD + QE + IJKL + UO + NM
ForcepControl forcep2Control;  // 方向鍵 + PageUp/PageDown + 數字鍵

glm::vec3 HITPOSITION = glm::vec3(0.0f, 0.0f, 0.0f); // 碰撞點位置
// UI manager
std::unique_ptr<UIManager> uiManager;

// Callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void forcepKeyboard(GLFWwindow* window);
void processInput(GLFWwindow* window);
void checkCollisionWithWound(btCollisionWorld& world, const std::vector<std::shared_ptr<Forcep>>& forceps, Model* wound);
void checkCollisionWithVessel(btCollisionWorld& world, const std::vector<std::shared_ptr<Forcep>>& forceps, Model* vessel);
void checkCollisionForceps(btCollisionWorld& world, const std::vector<std::shared_ptr<Forcep>>& forceps);


int main() {
    static float lastCollisionTime = 0.0f;
    static bool forceEnabled = true;

    // 初始化 GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = nullptr;
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
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

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
        return -1;
    }

    // 配置 OpenGL
    glEnable(GL_DEPTH_TEST);

    // Initialize UI Manager with actual screen size
    uiManager = std::make_unique<UIManager>(ACTUAL_WIDTH, ACTUAL_HEIGHT);
    if (!uiManager->initialize()) {
        std::cout << "Failed to initialize UI Manager" << std::endl;
        return -1;
    }

    // Load shaders
    Shader ourShader("vertex_shader.glsl", "fragment_shader.glsl");

    // 創建兩個夾子
    auto forcep1 = std::make_shared<Forcep>(glm::vec3(-2.0f, 3.0f, 0.0f), glm::vec3(0.0f, 0.0f, -70.0f), 0.05f);
    auto forcep2 = std::make_shared<Forcep>(glm::vec3(2.0f, 3.0f, 0.0f), glm::vec3(0.0f, 0.0f, -110.0f), 0.05f);

    if (hapticTESTMODE) {
        // Enable haptic test mode
        if (!forcep1->haptic->initialize("Left Device")) {
            std::cout << "Haptic device initialization failed L" << std::endl;
            return -1;
        }
        if (!forcep2->haptic->initialize("Right Device")) {
            std::cout << "Haptic device initialization failed R" << std::endl;
            return -1;
        }
        hdStartScheduler();
        uiManager->updateMode("Haptic Mode");
        uiManager->showMessage("Haptic Device Enabled", 3.0f, glm::vec3(0.5f, 1.0f, 0.5f));
    }
    else {
        uiManager->updateMode("Keyboard Mode");
        uiManager->showMessage("Using Keyboard Control Mode", 3.0f, glm::vec3(0.5f, 0.5f, 1.0f));
    }



    // 設定夾子的鉗爪偏移（根據實際模型調整）
    forcep1->setClawUpperOffset(glm::vec3(0.0f, 0.0f, 0.0f));
    forcep1->setClawLowerOffset(glm::vec3(0.0f, 0.0f, 0.0f));
    forcep2->setClawUpperOffset(glm::vec3(0.0f, 0.0f, 0.0f));
    forcep2->setClawLowerOffset(glm::vec3(0.0f, 0.0f, 0.f));
    auto wound = std::make_shared<Model>(".\\src\\model\\PDA_noPDA_3\\PDA_noPDA_3", 1.0f, glm::vec3(0, -2.0f, 0.0f), glm::vec3(-90.0f, 90.0f, 0.0f));
    auto wound_mask = std::make_shared<Model>(".\\src\\model\\PDA_noPDA_3\\PDA_noPDA_3", 1.0f, glm::vec3(0, wound->getWorldPosition().y + 0.0f, 0.0f), glm::vec3(-90.0f, 90.0f, 0.0f));
    auto vessel = std::make_shared<Model>(".\\src\\model\\PDA_with_aorta_no_PA_2\\PDA_with_aorta_no_PA_2_long5", 0.1f, glm::vec3(0, -1.5f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    auto clip = std::make_shared<Model>(".\\src\\model\\PDA_with_aorta_no_PA_2\\PDA_with_aorta_no_PA_2_clip", 0.1f, glm::vec3(0, -1.5f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    auto vessel_squeeze = std::make_shared<Model>(".\\src\\model\\PDA_with_aorta_no_PA_2\\PDA_with_aorta_no_PA_2_squeeze", 0.1f, glm::vec3(0, -1.5f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));
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

        // 顯示 UI 說明
        uiManager->setInstructionsVisible(true);
    }

    // 處理 Bullet 碰撞檢測
    static btDefaultCollisionConfiguration collisionConfig;
    static btCollisionDispatcher dispatcher(&collisionConfig);
    static btDbvtBroadphase broadphase;
    static btCollisionWorld collisionWorld(&dispatcher, &broadphase, &collisionConfig);


    // 渲染循環
    while (!glfwWindowShouldClose(window)) {
        std::cout << "woundPOS: " << wound->getWorldPosition().x << ", "
            << wound->getWorldPosition().y << ", "
            << wound->getWorldPosition().z << std::endl;
        if (forcep1->getWorldPosition().y < vessel->getWorldRotation().y && !success) {
            success = true;
            uiManager->showMessage("Vessel Squeezed Successfully!", 3.0f, glm::vec3(0.5f, 1.0f, 0.5f));
            vessel_squeeze->setLocalPosition(glm::vec3((forcep1->clawHitpointL.x + forcep1->clawHitpointU.x) / 2, vessel->getWorldPosition().y, vessel->getWorldPosition().z));
            clip->setLocalPosition(glm::vec3((forcep1->clawHitpointL.x + forcep1->clawHitpointU.x) / 2, vessel->getWorldPosition().y, vessel->getWorldPosition().z));
        }
        // deltaTime calculation
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // FPS calculation
        static float fpsTimer = 0.0f;
        static int frameCount = 0;
        static float displayFPS = 0.0f;
        fpsTimer += deltaTime;
        frameCount++;
        if (fpsTimer >= 1.0f) {
            displayFPS = frameCount / fpsTimer;
            frameCount = 0;
            fpsTimer = 0.0f;
        }
        uiManager->updateFrameRate(displayFPS);

        // wound position update
        if (WOUND_MOVE) {
            static float t = 0.0f;
            t += deltaTime;
            float woundy = 0.5f * sin(t); // y 在 -0.5 ~ +0.5 之間
            wound->setWorldPosition(glm::vec3(wound->getWorldPosition().x, wound->oPos.y + woundy, wound->getWorldPosition().z));
            wound_mask->setWorldPosition(glm::vec3(wound->getWorldPosition().x, wound_mask->oPos.y + woundy, wound->getWorldPosition().z));
            if (success) {
                vessel_squeeze->setWorldPosition(glm::vec3(vessel_squeeze->getWorldPosition().x, vessel_squeeze->oPos.y + woundy, vessel_squeeze->getWorldPosition().z));
            }
            else {
                vessel->setWorldPosition(glm::vec3(vessel->getWorldPosition().x, vessel->oPos.y + woundy, vessel->getWorldPosition().z));
            }
        }
        processInput(window);

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


        // Reset forcep
        if (rPressed) {
            forcep1->reset();
            forcep2->reset();
            success = false;
        }

        // UI data update
        uiManager->updateForcepPosition(1, forcep1->getWorldPosition());
        uiManager->updateForcepPosition(2, forcep2->getWorldPosition());
        uiManager->updateForcepRotation(1, forcep1->getLocalRotation());
        uiManager->updateForcepRotation(2, forcep2->getLocalRotation());
        uiManager->updateClawAngle(1, forcep1->getClawAngle());
        uiManager->updateClawAngle(2, forcep2->getClawAngle());

        // Render
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (true) {
            // 使用著色器
            ourShader.use();

            // 設定投影矩陣
            glm::mat4 projection = glm::perspective(glm::radians(camera.zoom),
                (float)ACTUAL_WIDTH / (float)ACTUAL_HEIGHT,
                0.1f, 100.0f);
            ourShader.setMat4("projection", projection);

            // 設定視圖矩陣
            glm::mat4 view = camera.getViewMatrix();
            ourShader.setMat4("view", view);

            // 設定光照
            ourShader.setVec3("lightPos", camera.position);
            ourShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
            ourShader.setVec3("viewPos", camera.position);

            // 渲染夾子1 (紅色)
            ourShader.setVec3("objectColor", 1.0f, 0.3f, 0.3f);
            forcep1->Draw(ourShader);

            // 渲染夾子2 (藍色)
            ourShader.setVec3("objectColor", 0.3f, 0.3f, 1.0f);
            forcep2->Draw(ourShader);

            ourShader.setVec3("objectColor", 0.8f, 0.3f, 0.3f);
            //wound->Draw(ourShader);
            //wound_mask->Draw(ourShader);
			if (success) {
				vessel_squeeze->Draw(ourShader);
			}
            else {
                vessel->Draw(ourShader);
            }
            DrawSquare(forcep1->getWorldPosition(), 0.3f, glm::vec3(1, 0, 0), view, projection);
            //std::cout << "clawUpperPeak: OUT " << forcep1->clawUpperPeak.x << ", " << forcep1->clawUpperPeak.y << ", " << forcep1->clawUpperPeak.z << std::endl;
            DrawSquare(forcep1->clawUpperPeak, 0.3f, glm::vec3(0, 1, 0), view, projection);
            DrawSquare(forcep1->clawLowerPeak, 0.3f, glm::vec3(0, 0, 1), view, projection);
            DrawSquare(forcep1->clawHitpointL, 0.3f, glm::vec3(0.3, 0.3, 0.3), view, projection);
            DrawSquare(forcep1->clawHitpointU, 0.3f, glm::vec3(0.3, 0.3, 0.3), view, projection);
            DrawSquare(forcep2->getWorldPosition(), 0.3f, glm::vec3(1, 0, 0), view, projection);
            DrawSquare(forcep2->clawUpperPeak, 0.3f, glm::vec3(0, 1, 0), view, projection);
            DrawSquare(forcep2->clawLowerPeak, 0.3f, glm::vec3(0, 0, 1), view, projection);

            DrawSquare(HITPOSITION, 0.3f, glm::vec3(1, 0, 0), view, projection);
        }


        // Bullet Mesh update
        wound->updateMesh(&collisionWorld);
        //wound_mask->updateMesh(&collisionWorld);
        if (success) {
            vessel_squeeze->updateMesh(&collisionWorld);
        }
        else {
            vessel->updateMesh(&collisionWorld);
        }
        wound->bulletCollisionObject->setWorldTransform(btTransform(btQuaternion(0, 0, 0, 1), btVector3(wound->getWorldPosition().x, wound->getWorldPosition().y, wound->getWorldPosition().z)));
        //wound_mask->bulletCollisionObject->setWorldTransform(btTransform(btQuaternion(0, 0, 0, 1), btVector3(wound_mask->getWorldPosition().x, wound_mask->getWorldPosition().y, wound_mask->getWorldPosition().z)));
        if (success) {
            vessel_squeeze->bulletCollisionObject->setWorldTransform(btTransform(btQuaternion(0, 0, 0, 1), btVector3(vessel_squeeze->getWorldPosition().x, vessel_squeeze->getWorldPosition().y, vessel_squeeze->getWorldPosition().z)));
        }
        else {
            vessel->bulletCollisionObject->setWorldTransform(btTransform(btQuaternion(0, 0, 0, 1), btVector3(vessel->getWorldPosition().x, vessel->getWorldPosition().y, vessel->getWorldPosition().z)));
        }
        
        forcep1->updateMesh(&collisionWorld);
        forcep2->updateMesh(&collisionWorld);

		/*forcep1->updateRealMesh(&collisionWorld);
		forcep2->updateRealMesh(&collisionWorld);*/

        // Collision update
        if (hapticTESTMODE && currentState == GameState::GAME) {
            forcep1->hit = false;
            forcep2->hit = false;
            forcep1->hitDeep = false;
            forcep2->hitDeep = false;
            std::vector<std::shared_ptr<Forcep>> forceps = { forcep1 , forcep2};
            //checkCollisionWithWound(collisionWorld, forceps, wound.get());
			//checkCollisionForceps(collisionWorld, forceps);
            if (!success) {
                checkCollisionWithVessel(collisionWorld, forceps, vessel.get());
			}
			else {
				checkCollisionWithVessel(collisionWorld, forceps, vessel_squeeze.get());
			}
            forcep1->updateCollision();
            forcep2->updateCollision();
        }

        // UI update
        static float blinkTime = 0.0f;
		blinkTime += deltaTime;
        if (currentState == GameState::MENU && blinkTime >= 2.0f) {
            uiManager->showMenu("Press \"Space\" to start", 3.0f, glm::vec3(1.0f, 1.0f, 1.0f));
			blinkTime = 0; // 重置計時器
        }
        uiManager->updateCollisionStatus(forcep1->hit);
        uiManager->update(deltaTime);
        uiManager->render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glfwTerminate();
    return 0;
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

// 輸入處理
void processInput(GLFWwindow* window) {

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

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
            // 這裡可以添加跟隨模式切換邏輯
            uiManager->showMessage("Follow Mode Toggled", 2.0f, glm::vec3(0.5f, 1.0f, 0.5f));
            tPressed = true;
        }
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE) {
            tPressed = false;
        }

        // 重置夾子位置
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !rPressed) {
            // 重置邏輯可以在這裡添加
            uiManager->showMessage("Forcep Position Reset", 2.0f, glm::vec3(1.0f, 0.5f, 0.5f));
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
			//uiManager->hideMenu();
			uiManager->showMessage("Game Started", 2.0f, glm::vec3(0.5f, 1.0f, 0.5f));
			InstructionON = false;
			success = false; // 重置成功狀態
		}
	}
}

// 視窗大小改變回調
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    if (uiManager) {
        uiManager->setScreenSize(width, height);
    }
}

// 滑鼠移動回調
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

// 滾輪回調
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	if (currentState != GameState::GAME) return; 
    camera.zoom -= (float)yoffset;
    if (camera.zoom < 1.0f)
        camera.zoom = 1.0f;
    if (camera.zoom > 45.0f)
        camera.zoom = 45.0f;
}

void checkCollisionWithWound(btCollisionWorld& world, const std::vector<std::shared_ptr<Forcep>>& forceps, Model* wound) {
    if(currentState != GameState::GAME) return;
    for (auto& f : forceps) {
        if (!f->objU || !f->objL || f->hit) continue;
        std::vector<btCapsuleOBJ*> objs = { f->objU, f->objL };
        for (auto& obj : objs) {
            MyResult2Callback result(obj->Trans);
            world.contactPairTest(&(obj->Object), wound->bulletCollisionObject, result);
            if (result.hit && !f->hit) {
                f->hit = true;
                //std::cout << "UPOS: " << f->getWorldPosition().x << ", " << f->getWorldPosition().y << ", " << f->getWorldPosition().z << std::endl;
                //std::cout << "prePOS: " << f->prePos.x << ", " << f->prePos.y << ", " << f->prePos.z << std::endl;
                glm::vec3 dir = f->prePos - f->getWorldPosition();
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
                    hduVector3Dd force(f->speed.x, f->speed.y, f->speed.z);
                    f->force = force * (-1.0f); // 設定回饋力
                    f->prePos += f->speed;
                }
                else {
                    rate1 = dirALL / 0.02f;
                    rate2 = 0.5f / dirALL;
                    rate = rate1 + rate2;
                    hduVector3Dd force(dir.x * rate, dir.y * rate, dir.z * rate);
                    f->force = force * 1.0f;
                }



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
                    float rate = 500.0f;
                    /*hduVector3Dd force(pushDir.x* rate, pushDir.y * rate, pushDir.z * rate);
                    std::cout << "Push Dir: " << pushDir.x << ", " << pushDir.y << ", " << pushDir.z << std::endl;
                    f->force += force;*/
                }
                //std::cout << "Force: " << f->force << std::endl;

            }
        }
    }
    wound->prePos = wound->getWorldPosition();

}

void checkCollisionWithWound_(btCollisionWorld& world, const std::vector<std::shared_ptr<Forcep>>& forceps, Model* wound) {

    for (auto& f : forceps) {
        if (!f->objU || !f->objL || f->hit) continue;
        std::vector<btCapsuleOBJ*> objs = { f->objU, f->objL };
        for (auto& obj : objs) {
            MyResult2Callback result(obj->Trans);
            world.contactPairTest(&(obj->Object), wound->bulletCollisionObject, result);
            if (result.hit && !f->hit) {
                f->hit = true;
                //std::cout << "UPOS: " << f->getWorldPosition().x << ", " << f->getWorldPosition().y << ", " << f->getWorldPosition().z << std::endl;
                //std::cout << "prePOS: " << f->prePos.x << ", " << f->prePos.y << ", " << f->prePos.z << std::endl;
                glm::vec3 Odir = f->prePos - f->getWorldPosition();
                glm::vec3 dir = dot(Odir, normalize(f->penetrateMask)) * normalize(f->penetrateMask);
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
                    hduVector3Dd force(f->speed.x, f->speed.y, f->speed.z);
                    f->force = force * (-1.0f); // 設定回饋力
                    f->prePos += f->speed;
                }
                else {
                    rate1 = dirALL / 0.02f;
                    rate2 = 0.5f / dirALL;
                    rate = rate1 + rate2;
                    hduVector3Dd force(dir.x * rate, dir.y * rate, dir.z * rate);
                    f->force = force * 1.0f;
                }



                //        else if (dirALL < 0.02f) {
                            //rate1 = 0.0f; // 當距離小於 0.02 時，回饋力為 0
                //        }

                std::cout << "dirALL: " << dirALL << std::endl;
                //std::cout << "rate: " << rate * dirALL << std::endl;

                f->prePos += (wound->getWorldPosition() - wound->prePos);
                glm::vec3 pushDir = wound->getWorldPosition() - wound->prePos;
                glm::vec3 forcepToWound = f->getWorldPosition() - wound->prePos;

                if (glm::dot(pushDir, forcepToWound) > 0) {
                    f->setRelatedWorldPosition(pushDir);
                    float rate = 500.0f;
                    /*hduVector3Dd force(pushDir.x* rate, pushDir.y * rate, pushDir.z * rate);
                    std::cout << "Push Dir: " << pushDir.x << ", " << pushDir.y << ", " << pushDir.z << std::endl;
                    f->force += force;*/
                }
                //std::cout << "Force: " << f->force << std::endl;

            }
        }
    }
    wound->prePos = wound->getWorldPosition();

}

void checkCollisionWithVessel(btCollisionWorld& world, const std::vector<std::shared_ptr<Forcep>>& forceps, Model* vessel) {
    if(currentState != GameState::GAME) return;
    for (auto& f : forceps) {
        if (!f->objU || !f->objL || f->hit) continue;
        std::vector<btCapsuleOBJ*> objs = { f->objU, f->objL };
        //std::vector<std::shared_ptr<Model>> objs = {f->clawUpper, f->clawLower};
        for (auto& obj : objs) {
            //std::cout << "check"<< std::endl;
            MyResultCallback result(obj->Trans);
            //MyResultCallback result(obj->trans);
            world.contactPairTest(&(obj->Object), vessel->bulletCollisionObject, result);
			//world.contactPairTest(obj->bulletCollisionObject, vessel->bulletCollisionObject, result);
            if (result.hit && !f->hit) {
				HITPOSITION = glm::vec3(result.hitPoint.x(), result.hitPoint.y(), result.hitPoint.z());
                f->hit = true;
                glm::vec3 dir = f->prePos - f->getWorldPosition();
                std::cout << "hit Dir: " << dir.x << ", " << dir.y << ", " << dir.z << std::endl;
                float dirALL = sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
                float rate1 = dirALL / 0.02f;
                float rate2 = 0.5f / dirALL;
                float rate = 0.0f;
                if (dirALL == 0)
                    rate1 = 0.0f; // 避免除以零
                rate = rate1 + rate2;

                hduVector3Dd force(dir.x * rate, dir.y * rate, dir.z * rate);
                f->force = force * 1.0f; // 設定回饋力
                f->prePos += (vessel->getWorldPosition() - vessel->prePos);
                glm::vec3 pushDir = vessel->getWorldPosition() - vessel->prePos;
                glm::vec3 forcepToVessel = f->getWorldPosition() - vessel->prePos;

                if (glm::dot(pushDir, forcepToVessel) > 0) {
                    f->setRelatedWorldPosition(pushDir);
                    float rate = 500.0f;
                    /*hduVector3Dd force(pushDir.x* rate, pushDir.y * rate, pushDir.z * rate);
                    std::cout << "Push Dir: " << pushDir.x << ", " << pushDir.y << ", " << pushDir.z << std::endl;
                    f->force += force;*/
                }
                std::cout << "Force: " << f->force << std::endl;

            }
        }
    }
    vessel->prePos = vessel->getWorldPosition();
}

void checkCollisionForceps(btCollisionWorld& world, const std::vector<std::shared_ptr<Forcep>>& forceps) {
    if (currentState != GameState::GAME) return;
    auto f1 = forceps[0];
    auto f2 = forceps[1];
	if (!f1->objU || !f1->objL || f1->hit || !f2->objU || !f2->objL || f2->hit) return;
    std::vector<btCapsuleOBJ*> objs1 = { f1->objU, f1->objL };
	std::vector<btCapsuleOBJ*> objs2 = { f2->objU, f2->objL };
    for (auto& obj : objs1) {
        MyResultCallback result(obj->Trans);
		for (auto& obj2 : objs2) {
			// 檢查兩個夾子之間的碰撞
			world.contactPairTest(&(obj->Object), &(obj2->Object), result);
            if (result.hit && !f1->hit) {
                HITPOSITION = glm::vec3(result.hitPoint.x(), result.hitPoint.y(), result.hitPoint.z());
				f1->hit = true;
				f2->hit = true;
				glm::vec3 dir1 = f1->prePos - f1->getWorldPosition();
				glm::vec3 dir2 = f2->prePos - f2->getWorldPosition();
				std::cout << "hit Dir1: " << dir1.x << ", " << dir1.y << ", " << dir1.z << std::endl;
				std::cout << "hit Dir2: " << dir2.x << ", " << dir2.y << ", " << dir2.z << std::endl;
				float dirALL1 = sqrt(dir1.x * dir1.x + dir1.y * dir1.y + dir1.z * dir1.z);
				float dirALL2 = sqrt(dir2.x * dir2.x + dir2.y * dir2.y + dir2.z * dir2.z);
				float rate11 = dirALL1 / 0.02f;
				float rate21 = dirALL2 / 0.02f;
				float rate12 = 0.5f / dirALL1;
				float rate22 = 0.5f / dirALL2;
                if (dirALL1 == 0)
                    rate12 = 0.0f;
				if (dirALL2 == 0)
					rate22 = 0.0f; 
				float rateALL1 = rate11 + rate12;
				float rateALL2 = rate21 + rate22;
				hduVector3Dd force1(dir1.x * rateALL1, dir1.y * rateALL1, dir1.z * rateALL1);
				hduVector3Dd force2(dir2.x * rateALL2, dir2.y * rateALL2, dir2.z * rateALL2);
				f1->force = force1 * 1.0f; 
				f2->force = force2 * 1.0f; 
                break;
            }
        }
    }
}
