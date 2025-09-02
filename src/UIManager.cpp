#include "UIManager.h"
#include <iostream>

UIManager::UIManager(unsigned int screenWidth, unsigned int screenHeight) 
    : screenWidth(screenWidth), screenHeight(screenHeight),
      forcep1Position(0.0f), forcep2Position(0.0f),
      forcep1Rotation(0.0f), forcep2Rotation(0.0f),
      forcep1ClawAngle(0.0f), forcep2ClawAngle(0.0f),
      currentMode(u8"觸覺控制模式"), isColliding1(false), isColliding2(false), frameRate(0.0f),
      messageTimer(0.0f), messageDuration(0.0f), messageColor(1.0f),
      showInstructions(false), showTutorial(true), currentTutorialStep(0), tutorialTimer(0.0f) {
    
    textRenderer = std::make_unique<TextRenderer>(screenWidth, screenHeight);
}

UIManager::~UIManager() = default;

bool UIManager::initialize() {
    if (!textRenderer->initialize("src/fonts/msjh.ttf", 24)) {
        std::cout << u8"字體渲染初始化失敗" << std::endl;
        return false;
    }
    
    setupTextBoxes();
    setupTutorialPanel();
    setupProgressIndicator();
    return true;
}

void UIManager::setupTextBoxes() {
    const float leftPanelX = 20.0f * ALLSCALE;
    const float leftPanelStartY = 60.0f * ALLSCALE;
    const float sectionSpacing = 40.0f * ALLSCALE;
    const float lineSpacing = 22.0f * ALLSCALE;
    const float lineSpacingSmall = 26.0f * ALLSCALE;
    const float wordSize = 1.0f * ALLSCALE;
    float currentY = leftPanelStartY;
    
    // 系統標題 - 專業醫療風格
    textRenderer->addTextBox("title", TextBox(u8"◦ 微創手術模擬訓練系統", glm::vec2(leftPanelX, currentY), 1.1f * wordSize, COLOR_TITLE));
    currentY += lineSpacing + 12.0f;

    // 系統狀態區塊
    textRenderer->addTextBox("mode", TextBox(u8"控制模式: 觸覺操作", glm::vec2(leftPanelX, currentY), 0.8f * wordSize, COLOR_PRIMARY));
    currentY += lineSpacingSmall;

    textRenderer->addTextBox("fps", TextBox(u8"系統幀率: 0 FPS", glm::vec2(leftPanelX, currentY), 0.7f * wordSize, COLOR_MEDIUM_GRAY));
    currentY += sectionSpacing;
    
    // 手術器械1狀態 - 專業術語
    textRenderer->addTextBox("forcep1_title", TextBox(u8"▶ 主手術鉗 (左手)", glm::vec2(leftPanelX, currentY), 0.85f * wordSize, COLOR_TOOL_1));
    currentY += lineSpacing;
    textRenderer->addTextBox("forcep1_pos", TextBox(u8"  位置座標: (0.00, 0.00, 0.00)", glm::vec2(leftPanelX, currentY), 0.7f * wordSize, COLOR_CLEAN_WHITE));
    currentY += lineSpacing;
    textRenderer->addTextBox("forcep1_rot", TextBox(u8"  旋轉角度: (0.00, 0.00, 0.00)", glm::vec2(leftPanelX, currentY), 0.7f * wordSize, COLOR_CLEAN_WHITE));
    currentY += lineSpacing;
    textRenderer->addTextBox("forcep1_claw", TextBox(u8"  鉗口開度: 0.00°", glm::vec2(leftPanelX, currentY), 0.7f * wordSize, COLOR_CLEAN_WHITE));
    currentY += lineSpacing;
    textRenderer->addTextBox("collision1", TextBox(u8"  接觸狀態: 正常", glm::vec2(leftPanelX, currentY), 0.75f * wordSize, COLOR_SUCCESS));
    currentY += lineSpacing;
    textRenderer->addTextBox("force1", TextBox(u8"  觸覺回饋: (0.00, 0.00, 0.00) N", glm::vec2(leftPanelX, currentY), 0.75f * wordSize, COLOR_INFO));
    currentY += sectionSpacing;
    
    // 手術器械2狀態
    textRenderer->addTextBox("forcep2_title", TextBox(u8"▶ 輔助手術鉗 (右手)", glm::vec2(leftPanelX, currentY), 0.85f * wordSize, COLOR_TOOL_2));
    currentY += lineSpacing;
    textRenderer->addTextBox("forcep2_pos", TextBox(u8"  位置座標: (0.00, 0.00, 0.00)", glm::vec2(leftPanelX, currentY), 0.7f * wordSize, COLOR_CLEAN_WHITE));
    currentY += lineSpacing;
    textRenderer->addTextBox("forcep2_rot", TextBox(u8"  旋轉角度: (0.00, 0.00, 0.00)", glm::vec2(leftPanelX, currentY), 0.7f * wordSize, COLOR_CLEAN_WHITE));
    currentY += lineSpacing;
    textRenderer->addTextBox("forcep2_claw", TextBox(u8"  鉗口開度: 0.00°", glm::vec2(leftPanelX, currentY), 0.7f * wordSize, COLOR_CLEAN_WHITE));
    currentY += lineSpacing;
    textRenderer->addTextBox("collision2", TextBox(u8"  接觸狀態: 正常", glm::vec2(leftPanelX, currentY), 0.75f * wordSize, COLOR_SUCCESS));
    currentY += lineSpacing;
    textRenderer->addTextBox("force2", TextBox(u8"  觸覺回饋: (0.00, 0.00, 0.00) N", glm::vec2(leftPanelX, currentY), 0.75f * wordSize, COLOR_INFO));
    currentY += sectionSpacing;
    
    // 中央提示訊息區
    const float messageY = 50.0f;
    textRenderer->addTextBox("message", TextBox(u8"", glm::vec2(screenWidth / 2, messageY), 1.3f * wordSize, COLOR_HIGHLIGHT));
    textRenderer->setTextBoxVisible("message", false);
    
    // 操作指南
    setupGameFlowInstructions();
}

void UIManager::setupGameFlowInstructions() {
    const float rightPanelXRatio = 1.0f - 0.22f * ALLSCALE;
    const float rightPanelStartY = 60.0f * ALLSCALE;
    const float sectionSpacing = 32.0f * ALLSCALE;
    const float lineSpacing = 22.0f * ALLSCALE;
    const float wordSize = 1.0f * ALLSCALE;

    float rightX = screenWidth * rightPanelXRatio;
    float currentY = rightPanelStartY;
    
    // 操作指南標題
    textRenderer->addTextBox("inst_title", TextBox(u8"◦ 觸覺操作指南", glm::vec2(rightX, currentY), 1.0f * wordSize, COLOR_TITLE));
    currentY += sectionSpacing;

    // 基本操作說明
    textRenderer->addTextBox("inst_basic", TextBox(u8"▶ 基本操作", glm::vec2(rightX, currentY), 0.85f * wordSize, COLOR_PRIMARY_LIGHT));
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_grip", TextBox(u8"  • 握持觸覺手把進行操作", glm::vec2(rightX, currentY), 0.75f * wordSize, COLOR_CLEAN_WHITE));
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_move", TextBox(u8"  • 移動手把控制手術鉗位置", glm::vec2(rightX, currentY), 0.75f * wordSize, COLOR_CLEAN_WHITE));
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_button", TextBox(u8"  • 按下按鈕開合鉗口", glm::vec2(rightX, currentY), 0.75f * wordSize, COLOR_CLEAN_WHITE));
    currentY += sectionSpacing;

    // 手術流程
    textRenderer->addTextBox("inst_procedure", TextBox(u8"▶ 手術流程", glm::vec2(rightX, currentY), 0.85f * wordSize, COLOR_MEDICAL_GREEN));
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_step1", TextBox(u8"  1. 熟悉觸覺回饋感受", glm::vec2(rightX, currentY), 0.75f * wordSize, COLOR_CLEAN_WHITE));
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_step2", TextBox(u8"  2. 定位目標血管位置", glm::vec2(rightX, currentY), 0.75f * wordSize, COLOR_CLEAN_WHITE));
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_step3", TextBox(u8"  3. 精確夾持血管組織", glm::vec2(rightX, currentY), 0.75f * wordSize, COLOR_CLEAN_WHITE));
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_step4", TextBox(u8"  4. 完成手術目標", glm::vec2(rightX, currentY), 0.75f * wordSize, COLOR_CLEAN_WHITE));
    currentY += sectionSpacing;

    // 安全提示
    textRenderer->addTextBox("inst_safety", TextBox(u8"▶ 安全提示", glm::vec2(rightX, currentY), 0.85f * wordSize, COLOR_WARNING));
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_gentle", TextBox(u8"  • 輕柔操作，避免過度用力", glm::vec2(rightX, currentY), 0.75f * wordSize, COLOR_CLEAN_WHITE));
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_feedback", TextBox(u8"  • 注意觸覺回饋強度", glm::vec2(rightX, currentY), 0.75f * wordSize, COLOR_CLEAN_WHITE));

    setInstructionsVisible(false);
}

void UIManager::setupTutorialPanel() {
    const float tutorialX = screenWidth * 0.3f;
    const float tutorialY = screenHeight * 0.3f;
    const float wordSize = 1.0f * ALLSCALE;
    
    // 教學面板背景提示
    textRenderer->addTextBox("tutorial_bg", TextBox(u8"", glm::vec2(tutorialX - 50, tutorialY - 50), 0.8f * wordSize, COLOR_STERILE_BLUE));
    
    // 教學步驟標題
    textRenderer->addTextBox("tutorial_title", TextBox(u8"✦ 操作教學", glm::vec2(tutorialX, tutorialY), 1.2f * wordSize, COLOR_TITLE));
    
    // 教學內容
    textRenderer->addTextBox("tutorial_content", TextBox(u8"歡迎使用微創手術模擬系統", glm::vec2(tutorialX, tutorialY + 40), 0.9f * wordSize, COLOR_PRIMARY));
    
    // 教學提示
    textRenderer->addTextBox("tutorial_hint", TextBox(u8"請跟隨螢幕指示進行操作", glm::vec2(tutorialX, tutorialY + 70), 0.8f * wordSize, COLOR_MEDIUM_GRAY));
    
    // 預設隱藏教學面板
    textRenderer->setTextBoxVisible("tutorial_bg", false);
    textRenderer->setTextBoxVisible("tutorial_title", false);
    textRenderer->setTextBoxVisible("tutorial_content", false);
    textRenderer->setTextBoxVisible("tutorial_hint", false);
}

void UIManager::setupProgressIndicator() {
    const float progressX = 20.0f * ALLSCALE;
    const float progressY = screenHeight - 100.0f * ALLSCALE;
    const float wordSize = 1.0f * ALLSCALE;
    
    // 進度指示器
    textRenderer->addTextBox("progress_title", TextBox(u8"◦ 訓練進度", glm::vec2(progressX, progressY), 0.8f * wordSize, COLOR_TITLE));
    textRenderer->addTextBox("progress_bar", TextBox(u8"████████████", glm::vec2(progressX, progressY + 25), 0.7f * wordSize, COLOR_SUCCESS));
    textRenderer->addTextBox("progress_text", TextBox(u8"準備中...", glm::vec2(progressX, progressY + 45), 0.75f * wordSize, COLOR_MEDIUM_GRAY));
}


void UIManager::update(float deltaTime) {
    updateStatusDisplay();
    updateTutorial(deltaTime);
    updateMessage(deltaTime);
}

void UIManager::render() {
    textRenderer->renderAllTextBoxes();
}

void UIManager::updateStatusDisplay() {
    // 更新主手術鉗狀態
    textRenderer->updateTextBox("forcep1_pos", "  位置座標: " + formatVec3(forcep1Position));
    textRenderer->updateTextBox("forcep1_rot", "  旋轉角度: " + formatVec3(forcep1Rotation));
    textRenderer->updateTextBox("forcep1_claw", "  鉗口開度: " + formatFloat(forcep1ClawAngle) + "°");

    std::string collisionText1 = isColliding1 ? "  接觸狀態: 組織接觸" : "  接觸狀態: 正常";
    glm::vec3 collisionColor1 = isColliding1 ? COLOR_WARNING : COLOR_SUCCESS;
    textRenderer->updateTextBox("collision1", collisionText1);
    textRenderer->updateTextBoxColor("collision1", collisionColor1);

    // 更新輔助手術鉗狀態
    textRenderer->updateTextBox("forcep2_pos", "  位置座標: " + formatVec3(forcep2Position));
    textRenderer->updateTextBox("forcep2_rot", "  旋轉角度: " + formatVec3(forcep2Rotation));
    textRenderer->updateTextBox("forcep2_claw", "  鉗口開度: " + formatFloat(forcep2ClawAngle) + "°");

    // 更新控制模式
    textRenderer->updateTextBox("mode", "控制模式: " + currentMode);

    // 更新系統幀率
    textRenderer->updateTextBox("fps", "系統幀率: " + formatFloat(frameRate, 0) + " FPS");

    // 更新輔助手術鉗碰撞狀態
    std::string collisionText2 = isColliding2 ? "  接觸狀態: 組織接觸" : "  接觸狀態: 正常";
    glm::vec3 collisionColor2 = isColliding2 ? COLOR_WARNING : COLOR_SUCCESS;

    // 更新觸覺回饋
    textRenderer->updateTextBox("force1", "  觸覺回饋: " + formatVec3(force1) + " N");
    textRenderer->updateTextBox("force2", "  觸覺回饋: " + formatVec3(force2) + " N");

    textRenderer->updateTextBox("collision2", collisionText2);
    textRenderer->updateTextBoxColor("collision2", collisionColor2);
}

void UIManager::updateTutorial(float deltaTime) {
    if (!showTutorial) return;
    
    tutorialTimer += deltaTime;
    
    // 根據教學步驟更新內容
    std::string tutorialContent;
    std::string tutorialHint;
    
    switch (currentTutorialStep) {
        case 0:
            tutorialContent = u8"歡迎使用微創手術模擬系統";
            tutorialHint = u8"請握持觸覺操作手把開始體驗";
            break;
        case 1:
            tutorialContent = u8"感受觸覺回饋的力度變化";
            tutorialHint = u8"輕柔移動手把，體驗不同位置的觸感";
            break;
        case 2:
            tutorialContent = u8"學習精確控制手術鉗";
            tutorialHint = u8"嘗試按下按鈕開合鉗口";
            break;
        case 3:
            tutorialContent = u8"定位目標血管組織";
            tutorialHint = u8"將手術鉗移動至紅色血管位置";
            break;
        case 4:
            tutorialContent = u8"執行夾持操作";
            tutorialHint = u8"小心夾持血管，注意力度控制";
            break;
        default:
            tutorialContent = u8"教學完成，開始自由操作";
            tutorialHint = u8"您已準備好進行手術模擬訓練";
            showTutorial = false;
            break;
    }
    
    textRenderer->updateTextBox("tutorial_content", tutorialContent);
    textRenderer->updateTextBox("tutorial_hint", tutorialHint);
    
    // 自動進入下一教學步驟（可根據實際操作觸發）
    if (tutorialTimer > 5.0f) {
        currentTutorialStep++;
        tutorialTimer = 0.0f;
    }
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

    // Calculate centered text position
    glm::vec2 textSize = textRenderer->getTextSize(message, 1.5f);
    float centerX = (screenWidth - textSize.x) / 2.0f - 30;
    float centerY = (screenHeight - textSize.y) / 2.0f;

    textRenderer->updateTextBox("message", message);
    textRenderer->updateTextBoxPosition("message", glm::vec2(centerX, centerY * 0.8));
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
    float centerX = (screenWidth - textSize.x) / 2.0f;
    float centerY = (screenHeight - textSize.y) / 2.0f; // Center vertically

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

    // 設定所有指導元素的可見性
    textRenderer->setTextBoxVisible("inst_title", visible);
    textRenderer->setTextBoxVisible("inst_basic", visible);
    textRenderer->setTextBoxVisible("inst_grip", visible);
    textRenderer->setTextBoxVisible("inst_move", visible);
    textRenderer->setTextBoxVisible("inst_button", visible);
    textRenderer->setTextBoxVisible("inst_procedure", visible);
    textRenderer->setTextBoxVisible("inst_step1", visible);
    textRenderer->setTextBoxVisible("inst_step2", visible);
    textRenderer->setTextBoxVisible("inst_step3", visible);
    textRenderer->setTextBoxVisible("inst_step4", visible);
    textRenderer->setTextBoxVisible("inst_safety", visible);
    textRenderer->setTextBoxVisible("inst_gentle", visible);
    textRenderer->setTextBoxVisible("inst_feedback", visible);
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

// 教學系統控制方法
void UIManager::startTutorial() {
    showTutorial = true;
    currentTutorialStep = 0;
    tutorialTimer = 0.0f;
    setTutorialVisible(true);
}

void UIManager::nextTutorialStep() {
    if (showTutorial) {
        currentTutorialStep++;
        tutorialTimer = 0.0f;
        if (currentTutorialStep > 5) {
            showTutorial = false;
            setTutorialVisible(false);
        }
    }
}

void UIManager::setTutorialVisible(bool visible) {
    textRenderer->setTextBoxVisible("tutorial_bg", visible);
    textRenderer->setTextBoxVisible("tutorial_title", visible);
    textRenderer->setTextBoxVisible("tutorial_content", visible);
    textRenderer->setTextBoxVisible("tutorial_hint", visible);
}

void UIManager::showWelcomeScreen() {
    showMessage(u8"歡迎體驗微創手術模擬系統\n請跟隨指導進行操作", 5.0f, COLOR_TITLE);
    startTutorial();
}

void UIManager::showCompletionScreen() {
    showMessage(u8"恭喜完成手術模擬訓練！\n感謝您的體驗", 5.0f, COLOR_SUCCESS);
}

void UIManager::updateProgressBar(float progress) {
    int barLength = 12;
    int filledLength = static_cast<int>(progress * barLength);
    std::string progressBar = "";
    
    for (int i = 0; i < barLength; i++) {
        if (i < filledLength) {
            progressBar += u8"█";
        } else {
            progressBar += u8"░";
        }
    }
    
    textRenderer->updateTextBox("progress_bar", progressBar);
    
    std::string progressText;
    if (progress < 0.2f) {
        progressText = u8"準備中...";
    } else if (progress < 0.5f) {
        progressText = u8"學習操作中...";
    } else if (progress < 0.8f) {
        progressText = u8"執行手術中...";
    } else if (progress < 1.0f) {
        progressText = u8"即將完成...";
    } else {
        progressText = u8"訓練完成！";
    }
    
    textRenderer->updateTextBox("progress_text", progressText);
}

