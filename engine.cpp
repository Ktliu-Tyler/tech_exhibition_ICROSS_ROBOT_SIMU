#include "GameEngine.h"
#include <iostream>
#include <chrono>
#include <thread>

// 全域引擎指針（用於GLFW回調）
static GameEngine* g_engine = nullptr;

// ========================= 菜單場景實現 =========================
MenuScene::MenuScene(unsigned int screenWidth, unsigned int screenHeight) 
    : screenWidth(screenWidth), screenHeight(screenHeight), selectedOption(0), keyPressed(false) {
    currentState = GameState::MENU;
}

MenuScene::~MenuScene() = default;

void MenuScene::initialize() {
    uiManager = std::make_unique<UIManager>(screenWidth, screenHeight);
    if (!uiManager->initialize()) {
        std::cout << "Failed to initialize UI Manager for Menu" << std::endl;
    }
    setupMenuUI();
}

void MenuScene::setupMenuUI() {
    // 主標題
    uiManager->addTextBox("title", TextBox("=== SURGICAL SIMULATION ===", 
        glm::vec2(screenWidth/2 - 200, 100), 1.5f, glm::vec3(0.8f, 0.9f, 1.0f)));
    
    // 菜單選項
    uiManager->addTextBox("option1", TextBox("1. Start Simulation", 
        glm::vec2(screenWidth/2 - 100, 200), 1.0f, glm::vec3(1.0f, 1.0f, 1.0f)));
    
    uiManager->addTextBox("option2", TextBox("2. Settings", 
        glm::vec2(screenWidth/2 - 100, 250), 1.0f, glm::vec3(1.0f, 1.0f, 1.0f)));
    
    uiManager->addTextBox("option3", TextBox("3. Exit", 
        glm::vec2(screenWidth/2 - 100, 300), 1.0f, glm::vec3(1.0f, 1.0f, 1.0f)));
    
    // 控制說明
    uiManager->addTextBox("controls", TextBox("Use 1-3 keys to select, ENTER to confirm", 
        glm::vec2(screenWidth/2 - 150, 400), 0.8f, glm::vec3(0.7f, 0.7f, 0.7f)));
    
    updateSelection();
}

void MenuScene::updateSelection() {
    // 重置所有選項顏色
    uiManager->updateTextBoxColor("option1", glm::vec3(1.0f, 1.0f, 1.0f));
    uiManager->updateTextBoxColor("option2", glm::vec3(1.0f, 1.0f, 1.0f));
    uiManager->updateTextBoxColor("option3", glm::vec3(1.0f, 1.0f, 1.0f));
    
    // 高亮選中選項
    switch (selectedOption) {
        case 0:
            uiManager->updateTextBoxColor("option1", glm::vec3(1.0f, 1.0f, 0.0f));
            break;
        case 1:
            uiManager->updateTextBoxColor("option2", glm::vec3(1.0f, 1.0f, 0.0f));
            break;
        case 2:
            uiManager->updateTextBoxColor("option3", glm::vec3(1.0f, 1.0f, 0.0f));
            break;
    }
}

void MenuScene::update(float deltaTime) {
    uiManager->update(deltaTime);
}

void MenuScene::render() {
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    uiManager->render();
}

void MenuScene::processInput(GLFWwindow* window) {
    // 數字鍵選擇
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !keyPressed) {
        selectedOption = 0;
        updateSelection();
        keyPressed = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && !keyPressed) {
        selectedOption = 1;
        updateSelection();
        keyPressed = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && !keyPressed) {
        selectedOption = 2;
        updateSelection();
        keyPressed = true;
    }
    // 方向鍵選擇
    else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && !keyPressed) {
        selectedOption = (selectedOption - 1 + 3) % 3;
        updateSelection();
        keyPressed = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && !keyPressed) {
        selectedOption = (selectedOption + 1) % 3;
        updateSelection();
        keyPressed = true;
    }
    // 確認選擇
    else if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !keyPressed) {
        switch (selectedOption) {
            case 0:
                currentState = GameState::GAME;
                break;
            case 1:
                currentState = GameState::SETTINGS;
                break;
            case 2:
                currentState = GameState::EXIT;
                break;
        }
        keyPressed = true;
    }
    
    // 重置按鍵狀態
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE &&
        glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE &&
        glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE &&
        glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE &&
        glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE &&
        glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
        keyPressed = false;
    }
}

void MenuScene::cleanup() {
    uiManager.reset();
}

// ========================= 遊戲場景實現 =========================
GameScene::GameScene(unsigned int screenWidth, unsigned int screenHeight) 
    : screenWidth(screenWidth), screenHeight(screenHeight) {
    currentState = GameState::GAME;
}

GameScene::~GameScene() = default;

void GameScene::initialize() {
    uiManager = std::make_unique<UIManager>(screenWidth, screenHeight);
    if (!uiManager->initialize()) {
        std::cout << "Failed to initialize UI Manager for Game" << std::endl;
    }
    
    // 這裡初始化你的遊戲對象
    // 例如: forceps, models, camera 等等
    
    uiManager->showMessage("Game Started - Press ESC for menu", 3.0f, glm::vec3(0.5f, 1.0f, 0.5f));
}

void GameScene::update(float deltaTime) {
    // 這裡放入你現有的遊戲更新邏輯
    uiManager->update(deltaTime);
}

void GameScene::render() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // 這裡放入你現有的3D渲染邏輯
    
    uiManager->render();
}

void GameScene::processInput(GLFWwindow* window) {
    // ESC 回到菜單
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        currentState = GameState::MENU;
    }
    // P 鍵暫停
    else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        currentState = GameState::PAUSE;
    }
    
    // 這裡放入你現有的遊戲輸入處理邏輯
}

void GameScene::cleanup() {
    uiManager.reset();
    // 清理遊戲資源
}

// ========================= 暫停場景實現 =========================
PauseScene::PauseScene(unsigned int screenWidth, unsigned int screenHeight) 
    : screenWidth(screenWidth), screenHeight(screenHeight), selectedOption(0), keyPressed(false) {
    currentState = GameState::PAUSE;
}

PauseScene::~PauseScene() = default;

void PauseScene::initialize() {
    uiManager = std::make_unique<UIManager>(screenWidth, screenHeight);
    if (!uiManager->initialize()) {
        std::cout << "Failed to initialize UI Manager for Pause" << std::endl;
    }
    
    // 暫停菜單
    uiManager->addTextBox("title", TextBox("=== PAUSED ===", 
        glm::vec2(screenWidth/2 - 80, 100), 1.5f, glm::vec3(1.0f, 1.0f, 0.0f)));
    
    uiManager->addTextBox("option1", TextBox("1. Resume", 
        glm::vec2(screenWidth/2 - 50, 200), 1.0f, glm::vec3(1.0f, 1.0f, 1.0f)));
    
    uiManager->addTextBox("option2", TextBox("2. Main Menu", 
        glm::vec2(screenWidth/2 - 50, 250), 1.0f, glm::vec3(1.0f, 1.0f, 1.0f)));
    
    uiManager->addTextBox("controls", TextBox("Use 1-2 keys or UP/DOWN, ENTER to confirm", 
        glm::vec2(screenWidth/2 - 150, 350), 0.8f, glm::vec3(0.7f, 0.7f, 0.7f)));
}

void PauseScene::update(float deltaTime) {
    uiManager->update(deltaTime);
    
    // 更新選項高亮
    uiManager->updateTextBoxColor("option1", selectedOption == 0 ? 
        glm::vec3(1.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 1.0f, 1.0f));
    uiManager->updateTextBoxColor("option2", selectedOption == 1 ? 
        glm::vec3(1.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 1.0f, 1.0f));
}

void PauseScene::render() {
    glClearColor(0.0f, 0.0f, 0.0f, 0.8f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    uiManager->render();
}

void PauseScene::processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !keyPressed) {
        selectedOption = 0;
        keyPressed = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && !keyPressed) {
        selectedOption = 1;
        keyPressed = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && !keyPressed) {
        selectedOption = (selectedOption - 1 + 2) % 2;
        keyPressed = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && !keyPressed) {
        selectedOption = (selectedOption + 1) % 2;
        keyPressed = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !keyPressed) {
        currentState = selectedOption == 0 ? GameState::GAME : GameState::MENU;
        keyPressed = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !keyPressed) {
        currentState = GameState::GAME; // P鍵繼續遊戲
        keyPressed = true;
    }
    
    // 重置按鍵狀態
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE &&
        glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE &&
        glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE &&
        glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE &&
        glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE &&
        glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
        keyPressed = false;
    }
}

void PauseScene::cleanup() {
    uiManager.reset();
}

// ========================= 遊戲引擎實現 =========================
GameEngine::GameEngine(unsigned int width, unsigned int height, const char* title) 
    : window(nullptr), screenWidth(width), screenHeight(height), windowTitle(title),
      currentState(GameState::MENU), nextState(GameState::MENU),
      deltaTime(0.0f), lastFrame(0.0f) {
    g_engine = this;
}

GameEngine::~GameEngine() {
    shutdown();
}

bool GameEngine::initialize() {
    // 初始化 GLFW
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // 創建視窗
    window = glfwCreateWindow(screenWidth, screenHeight, windowTitle.c_str(), nullptr, nullptr);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    // 初始化 GLEW
    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return false;
    }
    
    // OpenGL 設定
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // 創建初始場景
    currentScene = createScene(currentState);
    if (currentScene) {
        currentScene->initialize();
    }
    
    return true;
}

void GameEngine::run() {
    while (!shouldClose() && currentState != GameState::EXIT) {
        // 計算 deltaTime
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // 處理輸入
        processInput();
        
        // 檢查狀態變化
        if (currentScene && currentScene->getNextState() != currentState) {
            changeState(currentScene->getNextState());
        }
        
        // 更新和渲染當前場景
        if (currentScene) {
            currentScene->update(deltaTime);
            currentScene->render();
        }
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void GameEngine::changeState(GameState newState) {
    if (newState == currentState) return;
    
    // 清理當前場景
    if (currentScene) {
        currentScene->cleanup();
    }
    
    // 創建新場景
    currentState = newState;
    currentScene = createScene(currentState);
    
    if (currentScene) {
        currentScene->initialize();
    }
}

std::unique_ptr<Scene> GameEngine::createScene(GameState state) {
    switch (state) {
        case GameState::MENU:
            return std::make_unique<MenuScene>(screenWidth, screenHeight);
        case GameState::GAME:
            return std::make_unique<GameScene>(screenWidth, screenHeight);
        case GameState::PAUSE:
            return std::make_unique<PauseScene>(screenWidth, screenHeight);
        case GameState::SETTINGS:
            // 暫時回到菜單，你可以實現設定場景
            return std::make_unique<MenuScene>(screenWidth, screenHeight);
        default:
            return nullptr;
    }
}

void GameEngine::processInput() {
    if (currentScene) {
        currentScene->processInput(window);
    }
}

bool GameEngine::shouldClose() const {
    return glfwWindowShouldClose(window);
}

void GameEngine::shutdown() {
    if (currentScene) {
        currentScene->cleanup();
        currentScene.reset();
    }
    
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    
    glfwTerminate();
}

// GLFW 回調函數
void GameEngine::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    if (g_engine) {
        g_engine->screenWidth = width;
        g_engine->screenHeight = height;
    }
}