#include "UIManager.h"
#include <iostream>

UIManager::UIManager(unsigned int screenWidth, unsigned int screenHeight) 
    : screenWidth(screenWidth), screenHeight(screenHeight),
      forcep1Position(0.0f), forcep2Position(0.0f),
      forcep1Rotation(0.0f), forcep2Rotation(0.0f),
      forcep1ClawAngle(0.0f), forcep2ClawAngle(0.0f),
      currentMode(u8"一般模式"), isColliding1(false), isColliding2(false), frameRate(0.0f),
      messageTimer(0.0f), messageDuration(0.0f), messageColor(1.0f),
      showInstructions(false) {
    
    textRenderer = std::make_unique<TextRenderer>(screenWidth, screenHeight);
}

UIManager::~UIManager() = default;

bool UIManager::initialize() {//("src/fonts/msjh.ttf", 24)
    if (!textRenderer->initialize("src/fonts/msjh.ttf", 24)) {
        std::cout << u8"字體渲染初始化失敗" << std::endl;
        return false;
    }
    
    setupTextBoxes();
    return true;
}

void UIManager::setupTextBoxes() {
    const float leftPanelX = 15.0f * ALLSCALE;
    const float leftPanelStartY = 50.0f * ALLSCALE;
    const float sectionSpacing = 35.0f * ALLSCALE;
    const float lineSpacing = 20.0f * ALLSCALE;
    const float lineSpacingSmall = 25.0f * ALLSCALE;
    const float wordSize = 1.0f * ALLSCALE;
    float currentY = leftPanelStartY;
    // 系統標題
    textRenderer->addTextBox("title", TextBox(u8"=== 手術模擬系統 ===", glm::vec2(leftPanelX, currentY), 1.5f * wordSize, COLOR_PRIMARY));
    currentY += lineSpacing + 25.0f;


    // 模式 & FPS
    textRenderer->addTextBox("mode", TextBox(u8"提示: ", glm::vec2(leftPanelX, currentY), 1.5f * wordSize, COLOR_WARNING));
    currentY += lineSpacingSmall;


    textRenderer->addTextBox("fps", TextBox(u8"幀率: 0", glm::vec2(leftPanelX, currentY), 0.7f * wordSize, COLOR_GRAY));
    currentY += sectionSpacing;
    // 鉗子1 狀態
    textRenderer->addTextBox("forcep1_title", TextBox(u8"=== 手術器具 ( 左 ) ===", glm::vec2(leftPanelX, currentY), 0.8f * wordSize, COLOR_FORCE1));
    currentY += lineSpacing;
    textRenderer->addTextBox("forcep1_pos", TextBox(u8"位置: (0.00, 0.00, 0.00)", glm::vec2(leftPanelX, currentY), 0.7f * wordSize, COLOR_WHITE));
    currentY += lineSpacing;
    textRenderer->addTextBox("forcep1_rot", TextBox(u8"旋轉: (0.00, 0.00, 0.00)", glm::vec2(leftPanelX, currentY), 0.7f * wordSize, COLOR_WHITE));
    currentY += lineSpacing;
    textRenderer->addTextBox("forcep1_claw", TextBox(u8"爪角度: 0.00 度", glm::vec2(leftPanelX, currentY), 0.7f * wordSize, COLOR_WHITE));
    currentY += lineSpacing;
    textRenderer->addTextBox("collision1", TextBox(u8"碰撞狀態: 無", glm::vec2(leftPanelX, currentY), 0.8f * wordSize, COLOR_SUCCESS));
    currentY += lineSpacing;
    textRenderer->addTextBox("force1", TextBox(u8"受力: (0.00, 0.00, 0.00) N", glm::vec2(leftPanelX, currentY), 0.8f * wordSize, COLOR_INFO));
    currentY += sectionSpacing;
    // 鉗子2 狀態
    textRenderer->addTextBox("forcep2_title", TextBox(u8"=== 手術器具 ( 右 ) ===", glm::vec2(leftPanelX, currentY), 0.8f * wordSize, COLOR_FORCE2));
    currentY += lineSpacing;
    textRenderer->addTextBox("forcep2_pos", TextBox(u8"位置: (0.00, 0.00, 0.00)", glm::vec2(leftPanelX, currentY), 0.7f * wordSize, COLOR_WHITE));
    currentY += lineSpacing;
    textRenderer->addTextBox("forcep2_rot", TextBox(u8"旋轉: (0.00, 0.00, 0.00)", glm::vec2(leftPanelX, currentY), 0.7f * wordSize, COLOR_WHITE));
    currentY += lineSpacing;
    textRenderer->addTextBox("forcep2_claw", TextBox(u8"爪角度: 0.00 度", glm::vec2(leftPanelX, currentY), 0.7f * wordSize, COLOR_WHITE));
    currentY += lineSpacing;
    textRenderer->addTextBox("collision2", TextBox(u8"碰撞狀態: 無", glm::vec2(leftPanelX, currentY), 0.8f * wordSize, COLOR_SUCCESS));
    currentY += lineSpacing;
    textRenderer->addTextBox("force2", TextBox(u8"受力: (0.00, 0.00, 0.00) N", glm::vec2(leftPanelX, currentY), 0.8f * wordSize, COLOR_INFO));
    currentY += sectionSpacing;
    // 中央提示訊息
    const float messageY = 50.0f;
    textRenderer->addTextBox("message", TextBox(u8"", glm::vec2(screenWidth / 2, messageY), 1.2f * wordSize, COLOR_WARNING));
    textRenderer->setTextBoxVisible("message", false);
    // 操作步驟
    setupInstructions();
}

void UIManager::setupInstructions() {
    const float rightPanelXRatio = 1.0f - 0.20 * ALLSCALE;
    const float rightPanelStartY = 50.0f * ALLSCALE;
    const float sectionSpacing = 38.0f * ALLSCALE;
    const float lineSpacing = 27.0f * ALLSCALE;
    const float wordSize = 1.0f * ALLSCALE;

    float rightX = screenWidth * rightPanelXRatio;
    float currentY = rightPanelStartY;
    
    //textRenderer->addTextBox("inst_title", TextBox(u8"==== 操作說明 ====", glm::vec2(rightX, currentY), 1.0f * wordSize, glm::vec3(0.8f, 0.9f, 1.0f)));
    //currentY += sectionSpacing;
    textRenderer->addTextBox("inst_step0", TextBox(u8"==== 操作說明 ====", glm::vec2(rightX, currentY), 1.5f * wordSize, COLOR_PRIMARY));
    currentY += sectionSpacing;
    textRenderer->addTextBox("inst_step1", TextBox(u8"將搖桿拔出後 上下有兩個按鈕", glm::vec2(rightX, currentY), 1.0f * wordSize, glm::vec3(1.0f, 1.0f, 1.0f)));
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_step2", TextBox(u8"  上: 按住即可移動搖桿進行控制", glm::vec2(rightX, currentY), 1.0f * wordSize, COLOR_WARNING));
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_step3", TextBox(u8"  下: 按住夾爪便會夾緊", glm::vec2(rightX, currentY), 1.0f * wordSize, COLOR_WARNING));
    currentY += lineSpacing;
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_step4", TextBox(u8"請根據右上方提示進行模擬", glm::vec2(rightX, currentY), 1.0f * wordSize, glm::vec3(1.0f, 1.0f, 1.0f)));
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_step5", TextBox(u8"按下 ECS 離開系統", glm::vec2(rightX, currentY), 1.0f * wordSize, glm::vec3(1.0f, 1.0f, 1.0f)));
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_step6", TextBox(u8"若需重置 請使用重置功能 按下 R 鍵", glm::vec2(rightX, currentY), 1.0f * wordSize, glm::vec3(1.0f, 1.0f, 1.0f)));
    currentY += lineSpacing;

    setInstructionsVisible(false);
}


void UIManager::update(float deltaTime) {
    updateStatusDisplay();
    updateMessage(deltaTime);
}

void UIManager::render() {
    textRenderer->renderAllTextBoxes();
}

void UIManager::updateStatusDisplay() {
    // 更新鉗子 1 狀態
    textRenderer->updateTextBox("forcep1_pos", "位置: " + formatVec3(forcep1Position));
    textRenderer->updateTextBox("forcep1_rot", "旋轉: " + formatVec3(forcep1Rotation));
    textRenderer->updateTextBox("forcep1_claw", "爪角度: " + formatFloat(forcep1ClawAngle) + " 度");

    std::string collisionText1 = isColliding1 ? "碰撞狀態: 碰撞中" : "碰撞狀態: 無";
    glm::vec3 collisionColor1 = isColliding1 ? COLOR_ERROR : COLOR_SUCCESS;
    textRenderer->updateTextBox("collision1", collisionText1);
    textRenderer->updateTextBoxColor("collision1", collisionColor1);

    // 更新鉗子 2 狀態
    textRenderer->updateTextBox("forcep2_pos", "位置: " + formatVec3(forcep2Position));
    textRenderer->updateTextBox("forcep2_rot", "旋轉: " + formatVec3(forcep2Rotation));
    textRenderer->updateTextBox("forcep2_claw", "爪角度: " + formatFloat(forcep2ClawAngle) + " 度");

    // 更新模式
    textRenderer->updateTextBox("mode", "提示: " + currentMode);

    // 更新幀率
    textRenderer->updateTextBox("fps", "幀率: " + formatFloat(frameRate, 0));

    // 更新鉗子 2 碰撞狀態
    std::string collisionText2 = isColliding2 ? "碰撞狀態: 碰撞中" : "碰撞狀態: 無";
    glm::vec3 collisionColor2 = isColliding2 ? COLOR_ERROR : COLOR_SUCCESS;

    // 更新受力
    textRenderer->updateTextBox("force1", "受力: " + formatVec3(force1));
    textRenderer->updateTextBox("force2", "受力: " + formatVec3(force2));

    textRenderer->updateTextBox("collision2", collisionText2);
    textRenderer->updateTextBoxColor("collision2", collisionColor2);
}


void UIManager::updateMessage(float deltaTime) {
    if (messageTimer > 0.0f) {
        messageTimer -= deltaTime;
        if (messageTimer <= 0.0f) {
            hideMessage();
        }
        else {
            // Fade out effect
            float alpha = messageTimer / messageDuration;
            glm::vec3 fadedColor = messageColor * alpha;
            textRenderer->updateTextBoxColor("message", fadedColor);
        }
    }
}

void UIManager::updateForcepPosition(int forcepId, const glm::vec3& position) {
    if (forcepId == 1) {
        forcep1Position = position;
    }
    else if (forcepId == 2) {
        forcep2Position = position;
    }
}

void UIManager::updateForce(int forcepId, const glm::vec3& force) {
    if (forcepId == 1) {
        force1 = force;
    }
    else if (forcepId == 2) {
        force2 = force;
    }
}

void UIManager::updateForcepRotation(int forcepId, const glm::vec3& rotation) {
    if (forcepId == 1) {
        forcep1Rotation = rotation;
    }
    else if (forcepId == 2) {
        forcep2Rotation = rotation;
    }
}

void UIManager::updateClawAngle(int forcepId, float angle) {
    if (forcepId == 1) {
        forcep1ClawAngle = angle;
    }
    else if (forcepId == 2) {
        forcep2ClawAngle = angle;
    }
}

void UIManager::updateMode(const std::string& mode) {
    currentMode = mode;
}

void UIManager::updateCollisionStatus(bool colliding1, bool colliding2) {
    isColliding1 = colliding1;
    isColliding2 = colliding2;
}

void UIManager::updateFrameRate(float fps) {
    frameRate = fps;
}

void UIManager::showMessage(const std::string& message, float duration, glm::vec3 color) {
    messageText = message;
    messageDuration = duration;
    messageTimer = duration;
    messageColor = color;
    const float rightPanelXRatio = 1.0f - 0.80 * ALLSCALE;
    const float rightPanelStartY = 50.0f * ALLSCALE;
    // Calculate centered text position
    glm::vec2 textSize = textRenderer->getTextSize(message, 1.5f);
	
    float centerX = (screenWidth - 450.0f* message.length()/22.0f) / 2.0f;
	printf("textSize.x: %f\n", textSize.x);
    float centerY = 120.0f;

    textRenderer->updateTextBox("message", message);
    textRenderer->updateTextBoxPosition("message", glm::vec2(centerX, centerY));
    textRenderer->updateTextBoxColor("message", color);
    textRenderer->setTextBoxVisible("message", true);
}

void UIManager::showMenu(const std::string& message, float duration, glm::vec3 color) {
    messageText = message;
    messageDuration = duration;
    messageTimer = duration;
    messageColor = color;

    // Use larger scale for menu text
    float menuScale = 2.0f;  // Make menu text 2x larger

    // Calculate centered text position with larger scale
    glm::vec2 textSize = textRenderer->getTextSize(message, menuScale);
    float centerX = (screenWidth - 525.0f) / 2.0f;
    //float centerY = (screenHeight - textSize.y) / 2.0f; // Center vertically

    float centerY = 120.0f;

    textRenderer->updateTextBox("message", message);
    textRenderer->updateTextBoxPosition("message", glm::vec2(centerX, centerY));
    textRenderer->updateTextBoxColor("message", color);
    // Update the text box scale for menu display
    textRenderer->updateTextBoxScale("message", menuScale);
    textRenderer->setTextBoxVisible("message", true);
}

void UIManager::hideMessage() {
    textRenderer->setTextBoxVisible("message", false);
    messageTimer = 0.0f;
}

void UIManager::toggleInstructions() {
    showInstructions = !showInstructions;
    setInstructionsVisible(showInstructions);
}

void UIManager::setInstructionsVisible(bool visible) {
    showInstructions = visible;

    // Set visibility for all instruction text elements
    textRenderer->setTextBoxVisible("inst_title", visible);
    textRenderer->setTextBoxVisible("inst_mode", visible);
    textRenderer->setTextBoxVisible("inst_mode_switch", visible);
    textRenderer->setTextBoxVisible("inst_camera", visible);
    textRenderer->setTextBoxVisible("inst_mouse", visible);
    textRenderer->setTextBoxVisible("inst_scroll", visible);
    textRenderer->setTextBoxVisible("inst_haptic", visible);
    textRenderer->setTextBoxVisible("inst_haptic_move", visible);
    textRenderer->setTextBoxVisible("inst_haptic_button1", visible);
    textRenderer->setTextBoxVisible("inst_haptic_button2", visible);
    textRenderer->setTextBoxVisible("inst_haptic_rotate", visible);
    textRenderer->setTextBoxVisible("inst_keyboard", visible);
    textRenderer->setTextBoxVisible("inst_f1", visible);
    textRenderer->setTextBoxVisible("inst_f1_move", visible);
    textRenderer->setTextBoxVisible("inst_f1_rotate", visible);
    textRenderer->setTextBoxVisible("inst_f1_claw", visible);
    textRenderer->setTextBoxVisible("inst_f2", visible);
    textRenderer->setTextBoxVisible("inst_f2_move", visible);
    textRenderer->setTextBoxVisible("inst_f2_rotate", visible);
    textRenderer->setTextBoxVisible("inst_f2_claw", visible);
    textRenderer->setTextBoxVisible("inst_other", visible);
    textRenderer->setTextBoxVisible("inst_reset", visible);
    textRenderer->setTextBoxVisible("inst_help", visible);
    textRenderer->setTextBoxVisible("inst_exit", visible);
}

void UIManager::setScreenSize(unsigned int width, unsigned int height) {
    screenWidth = width;
    screenHeight = height;
    textRenderer->setScreenSize(width, height);
}

std::string UIManager::formatFloat(float value, int precision) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision) << value;
    return ss.str();
}

std::string UIManager::formatVec3(const glm::vec3& vec, int precision) {
    return "(" + formatFloat(vec.x, precision) + ", " +
        formatFloat(vec.y, precision) + ", " +
        formatFloat(vec.z, precision) + ")";
}

// 公共文字框管理方法
void UIManager::addTextBox(const std::string& id, const TextBox& textBox) {
    textRenderer->addTextBox(id, textBox);
}

void UIManager::updateTextBox(const std::string& id, const std::string& text) {
    textRenderer->updateTextBox(id, text);
}

void UIManager::updateTextBoxColor(const std::string& id, glm::vec3 color) {
    textRenderer->updateTextBoxColor(id, color);
}

void UIManager::updateTextBoxScale(const std::string& id, float scale) {
    textRenderer->updateTextBoxScale(id, scale);
}

void UIManager::setTextBoxVisible(const std::string& id, bool visible) {
    textRenderer->setTextBoxVisible(id, visible);
}

