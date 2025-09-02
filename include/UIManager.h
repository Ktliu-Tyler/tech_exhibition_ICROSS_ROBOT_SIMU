#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "Constant.h"
#include "TextRenderer.h"
#include <memory>
#include <sstream>
#include <iomanip>

// ==== 顏色調色盤 ====
// 基本色
const glm::vec3 COLOR_WHITE = glm::vec3(1.0f, 1.0f, 1.0f);
const glm::vec3 COLOR_GRAY = glm::vec3(0.7f, 0.7f, 0.7f);
const glm::vec3 COLOR_LIGHTGRAY = glm::vec3(0.85f, 0.85f, 0.85f);


// 主題色
const glm::vec3 COLOR_PRIMARY = glm::vec3(0.6f, 0.8f, 1.0f); // 系統標題 藍
const glm::vec3 COLOR_SECONDARY = glm::vec3(0.3f, 0.9f, 0.7f); // 醫療綠


// 狀態色
const glm::vec3 COLOR_WARNING = glm::vec3(0.95f, 0.85f, 0.2f); // 黃色 (模式/提示)
const glm::vec3 COLOR_ERROR = glm::vec3(1.0f, 0.3f, 0.3f); // 紅色 (碰撞)
const glm::vec3 COLOR_SUCCESS = glm::vec3(0.5f, 1.0f, 0.5f); // 綠色 (安全)


// 工具色
const glm::vec3 COLOR_FORCE1 = glm::vec3(1.0f, 0.4f, 0.4f); // 鉗子1 標題
const glm::vec3 COLOR_FORCE2 = glm::vec3(0.4f, 0.6f, 1.0f); // 鉗子2 標題
const glm::vec3 COLOR_INFO = glm::vec3(0.8f, 0.9f, 1.0f); // 資訊字體 淺藍


class UIManager {
public:
    UIManager(unsigned int screenWidth, unsigned int screenHeight);
    ~UIManager();
    
    bool initialize();
    void update(float deltaTime);
    void render();
    
    // 狀態顯示相關
    void updateForcepPosition(int forcepId, const glm::vec3& position);
    void updateForcepRotation(int forcepId, const glm::vec3& rotation);
    void updateClawAngle(int forcepId, float angle);
    void updateMode(const std::string& mode);
    void updateCollisionStatus(bool isColliding1, bool isColliding2);
    void updateFrameRate(float fps);
    void updateForce(int forcepId, const glm::vec3& force);
    
    // 提示訊息
    void showMessage(const std::string& message, float duration = 3.0f, glm::vec3 color = glm::vec3(1.0f, 1.0f, 0.0f));
    void showMenu(const std::string& message, float duration, glm::vec3 color);
    void hideMessage();
    
    // 控制說明
    void toggleInstructions();
    void setInstructionsVisible(bool visible);
    
    // 文字框管理（公共接口）
    void addTextBox(const std::string& id, const TextBox& textBox);
    void updateTextBox(const std::string& id, const std::string& text);
    void updateTextBoxColor(const std::string& id, glm::vec3 color);
    void updateTextBoxScale(const std::string& id, float scale);
    void setTextBoxVisible(const std::string& id, bool visible);
    
    // 工具函數
    void setupInstructions();
    void setScreenSize(unsigned int width, unsigned int height);
    std::string formatFloat(float value, int precision = 2);
    std::string formatVec3(const glm::vec3& vec, int precision = 2);

private:
    std::unique_ptr<TextRenderer> textRenderer;
    unsigned int screenWidth, screenHeight;
    
    // 狀態變數
    glm::vec3 forcep1Position, forcep2Position;
    glm::vec3 forcep1Rotation, forcep2Rotation;
    glm::vec3 force1, force2;

    float forcep1ClawAngle, forcep2ClawAngle;
    std::string currentMode;
    bool isColliding1, isColliding2;
    float frameRate;
    
    // 訊息系統
    std::string messageText;
    float messageTimer;
    float messageDuration;
    glm::vec3 messageColor;
    
    // UI狀態
    bool showInstructions;
    
    void setupTextBoxes();
    void updateStatusDisplay();
    void updateInstructions();
    void updateMessage(float deltaTime);
};

#endif
