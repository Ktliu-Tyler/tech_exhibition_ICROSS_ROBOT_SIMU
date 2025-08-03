#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <iostream>
#include "UIManager.h"

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

// 遊戲狀態枚舉
enum class GameState {
    MENU,           // 主菜單
    GAME,           // 遊戲進行中
    GAME_SUCCESS,
    PAUSE,          // 暫停畫面
    SETTINGS,       // 設定畫面
    EXIT            // 退出
};

// 遊戲引擎主類
class GameEngine {
public:
    GameEngine(unsigned int width, unsigned int height, const char* title);
    ~GameEngine();


    bool initialize();
    void run();
    void shutdown();
    
    // 狀態管理
    void changeState(GameState newState);
    GameState getCurrentState() const { return currentState; }
    
    bool shouldClose() const;

    void forcepKeyboard(GLFWwindow* window);
    void processInput(GLFWwindow* window);
    void checkCollisionWithWound(btCollisionWorld& world, const std::vector<std::shared_ptr<Forcep>>& forceps, Model* wound);
    void checkCollisionWithVessel(btCollisionWorld& world, const std::vector<std::shared_ptr<Forcep>>& forceps, Model* vessel);
    
//private:
    
    unsigned int screenWidth, screenHeight;
    std::string windowTitle;
    
    GameState currentState;
    GameState nextState;
    
    const unsigned int SCR_WIDTH = WIDTH;
    const unsigned int SCR_HEIGHT = HEIGHT;

    // Camera setup
	UIManager* uiManager;
    // Variables
    float lastX = WIDTH / 2.0f;
    float lastY = HEIGHT / 2.0f;

    bool mouseCONTROL = MOUSE_INVISIBLE; // 是否啟用滑鼠控制攝像機
    bool InstructionON = SHOW_INSTRUCTION; // 是否顯示控制說明
    bool hapticTESTMODE = HAPTIC_MODE; // 是否啟用觸覺測試模式
    bool success = true;

    bool tPressed = false;
    bool rPressed = false;
    bool f1Pressed = false;

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    bool firstMouse = true;

    ForcepControl forcep1Control;  // WASD + QE + IJKL + UO + NM
    ForcepControl forcep2Control;  // 方向鍵 + PageUp/PageDown + 數字鍵
};

// callback函數
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

#endif
