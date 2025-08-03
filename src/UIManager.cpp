#include "UIManager.h"
#include <iostream>

UIManager::UIManager(unsigned int screenWidth, unsigned int screenHeight) 
    : screenWidth(screenWidth), screenHeight(screenHeight),
      forcep1Position(0.0f), forcep2Position(0.0f),
      forcep1Rotation(0.0f), forcep2Rotation(0.0f),
      forcep1ClawAngle(0.0f), forcep2ClawAngle(0.0f),
      currentMode("Normal"), isColliding(false), frameRate(0.0f),
      messageTimer(0.0f), messageDuration(0.0f), messageColor(1.0f),
      showInstructions(false) {
    
    textRenderer = std::make_unique<TextRenderer>(screenWidth, screenHeight);
}

UIManager::~UIManager() = default;

bool UIManager::initialize() {
    // Initialize text renderer using Arial font
    if (!textRenderer->initialize("src/fonts/arial.ttf", 24)) {
        std::cout << "Failed to initialize text renderer" << std::endl;
        return false;
    }
    
    setupTextBoxes();
    return true;
}

void UIManager::setupTextBoxes() {
    // Layout parameters - adjust these to change overall positioning
    const float leftPanelX = 15.0f * ALLSCALE;          // Left panel X position
    const float leftPanelStartY = 50.0f * ALLSCALE;     // Left panel starting Y position
    const float sectionSpacing = 35.0f * ALLSCALE;      // Spacing between sections
    const float lineSpacing = 20.0f * ALLSCALE;         // Spacing between lines within sections
    const float lineSpacingSmall = 25.0f * ALLSCALE;    // Smaller spacing for sub-items
	const float wordSize = 1.0f * ALLSCALE;            // Base font size for text
    // Calculate Y positions
    float currentY = leftPanelStartY;
    
    // Status display area (top left)
    textRenderer->addTextBox("title", TextBox("=== Surgery Simulation System ===", glm::vec2(leftPanelX, currentY), 1.0f * wordSize, glm::vec3(0.8f, 0.9f, 1.0f)));
    currentY += lineSpacing + 10.0f;
    textRenderer->addTextBox("mode", TextBox("Mode: Normal", glm::vec2(leftPanelX, currentY), 0.8f * wordSize, glm::vec3(0.9f, 0.9f, 0.3f)));
    currentY += lineSpacingSmall;
    textRenderer->addTextBox("fps", TextBox("FPS: 0", glm::vec2(leftPanelX, currentY), 0.7f * wordSize, glm::vec3(0.7f, 0.7f, 0.7f)));
    currentY += sectionSpacing;
    
    // Forcep 1 status (left side)
    textRenderer->addTextBox("forcep1_title", TextBox("=== Forcep 1 (Red) ===", glm::vec2(leftPanelX, currentY), 0.8f * wordSize, glm::vec3(1.0f, 0.5f, 0.5f)));
    currentY += lineSpacing;
    textRenderer->addTextBox("forcep1_pos", TextBox("Position: (0.00, 0.00, 0.00)", glm::vec2(leftPanelX, currentY), 0.7f * wordSize, glm::vec3(1.0f, 1.0f, 1.0f)));
    currentY += lineSpacing;
    textRenderer->addTextBox("forcep1_rot", TextBox("Rotation: (0.00, 0.00, 0.00)", glm::vec2(leftPanelX, currentY), 0.7f * wordSize, glm::vec3(1.0f, 1.0f, 1.0f)));
    currentY += lineSpacing;
    textRenderer->addTextBox("forcep1_claw", TextBox("Claw Angle: 0.00 deg", glm::vec2(leftPanelX, currentY), 0.7f * wordSize, glm::vec3(1.0f, 1.0f, 1.0f)));
    currentY += sectionSpacing;
    
    // Forcep 2 status (left side)
    textRenderer->addTextBox("forcep2_title", TextBox("=== Forcep 2 (Blue) ===", glm::vec2(leftPanelX, currentY), 0.8f * wordSize, glm::vec3(0.5f, 0.5f, 1.0f)));
    currentY += lineSpacing;
    textRenderer->addTextBox("forcep2_pos", TextBox("Position: (0.00, 0.00, 0.00)", glm::vec2(leftPanelX, currentY), 0.7f * wordSize, glm::vec3(1.0f, 1.0f, 1.0f)));
    currentY += lineSpacing;
    textRenderer->addTextBox("forcep2_rot", TextBox("Rotation: (0.00, 0.00, 0.00)", glm::vec2(leftPanelX, currentY), 0.7f * wordSize, glm::vec3(1.0f, 1.0f, 1.0f)));
    currentY += lineSpacing;
    textRenderer->addTextBox("forcep2_claw", TextBox("Claw Angle: 0.00 deg", glm::vec2(leftPanelX, currentY), 0.7f * wordSize, glm::vec3(1.0f, 1.0f, 1.0f)));
    currentY += sectionSpacing;
    
    // Collision status
    textRenderer->addTextBox("collision", TextBox("Collision Status: None", glm::vec2(leftPanelX, currentY), 0.8f * wordSize, glm::vec3(0.5f, 1.0f, 0.5f)));
    
    // Message display (center top of screen)
    const float messageY = 50.0f;  // Adjust this to move message up/down
    textRenderer->addTextBox("message", TextBox("", glm::vec2(screenWidth/2, messageY), 1.2f * wordSize, glm::vec3(1.0f, 1.0f, 0.0f)));
    textRenderer->setTextBoxVisible("message", false);
    
    // Control instructions (right side)
    setupInstructions();
}

void UIManager::setupInstructions() {
    // Layout parameters - adjust these to change overall positioning
    const float rightPanelXRatio = 1.0f - 0.18 * ALLSCALE;    // Right panel position as ratio of screen width (0.75 = 75% from left)
    const float rightPanelStartY = 50.0f * ALLSCALE;    // Right panel starting Y position
    const float sectionSpacing = 30.0f * ALLSCALE;      // Spacing between sections
    const float lineSpacing = 20.0f * ALLSCALE;         // Spacing between lines within sections
    const float subItemIndent = 20.0f * ALLSCALE;       // Indentation for sub-items
    const float wordSize = 1.0f * ALLSCALE;
    // Calculate X position
    float rightX = screenWidth * rightPanelXRatio;
    float currentY = rightPanelStartY;
    
    // Control instructions title
    textRenderer->addTextBox("inst_title", TextBox("==== Controls Instruction====", glm::vec2(rightX, currentY), 1.0f * wordSize, glm::vec3(0.8f, 0.9f, 1.0f)));
    currentY += sectionSpacing;
    
    // Mode switching section
    textRenderer->addTextBox("inst_mode", TextBox("Control Mode:", glm::vec2(rightX, currentY), 0.8f * wordSize, glm::vec3(0.9f, 0.9f, 0.3f)));
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_mode_switch", TextBox("  T: Switch between Haptic/Keyboard", glm::vec2(rightX + subItemIndent, currentY), 0.7f * wordSize, glm::vec3(1.0f, 1.0f, 0.5f)));
    currentY += sectionSpacing;
    
    // Camera controls section
    textRenderer->addTextBox("inst_camera", TextBox("Camera Control:", glm::vec2(rightX, currentY), 0.8f * wordSize, glm::vec3(0.9f, 0.9f, 0.3f)));
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_mouse", TextBox("  Mouse: Rotate view", glm::vec2(rightX + subItemIndent, currentY), 0.7f * wordSize, glm::vec3(1.0f, 1.0f, 1.0f)));
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_scroll", TextBox("  Scroll: Zoom", glm::vec2(rightX + subItemIndent, currentY), 0.7f * wordSize, glm::vec3(1.0f, 1.0f, 1.0f)));
    currentY += sectionSpacing;
    
    // Haptic mode controls section
    textRenderer->addTextBox("inst_haptic", TextBox("Haptic Mode (Both Forceps):", glm::vec2(rightX, currentY), 0.8f * wordSize, glm::vec3(0.5f, 1.0f, 0.5f)));
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_haptic_move", TextBox("  Move Device: Control forcep", glm::vec2(rightX + subItemIndent, currentY), 0.7f * wordSize, glm::vec3(1.0f, 1.0f, 1.0f)));
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_haptic_button1", TextBox("  Button 1: Open/Close claw", glm::vec2(rightX + subItemIndent, currentY), 0.7f * wordSize, glm::vec3(1.0f, 1.0f, 1.0f)));
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_haptic_button2", TextBox("  Button 2: Position mode", glm::vec2(rightX + subItemIndent, currentY), 0.7f * wordSize, glm::vec3(1.0f, 1.0f, 1.0f)));
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_haptic_rotate", TextBox("  Rotate Device: Rotate forcep", glm::vec2(rightX + subItemIndent, currentY), 0.7f * wordSize, glm::vec3(1.0f, 1.0f, 1.0f)));
    currentY += sectionSpacing;
    
    // Keyboard mode controls section
    textRenderer->addTextBox("inst_keyboard", TextBox("Keyboard Mode:", glm::vec2(rightX, currentY), 0.8f * wordSize, glm::vec3(0.9f, 0.7f, 0.3f)));
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_f1", TextBox("Forcep 1 (Red) - WASD Controls:", glm::vec2(rightX, currentY), 0.7f * wordSize, glm::vec3(1.0f, 0.5f, 0.5f)));
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_f1_move", TextBox("  WASD: Move, Q/E: Up/Down", glm::vec2(rightX + subItemIndent, currentY), 0.7f * wordSize, glm::vec3(1.0f, 1.0f, 1.0f)));
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_f1_rotate", TextBox("  I/K: Pitch, J/L: Yaw, U/O: Roll", glm::vec2(rightX + subItemIndent, currentY), 0.7f * wordSize, glm::vec3(1.0f, 1.0f, 1.0f)));
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_f1_claw", TextBox("  N/M: Open/Close claw", glm::vec2(rightX + subItemIndent, currentY), 0.7f * wordSize, glm::vec3(1.0f, 1.0f, 1.0f)));
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_f2", TextBox("Forcep 2 (Blue) - Arrow Controls:", glm::vec2(rightX, currentY), 0.7f * wordSize, glm::vec3(0.5f, 0.5f, 1.0f)));
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_f2_move", TextBox("  Arrows: Move, PgUp/PgDn: Up/Down", glm::vec2(rightX + subItemIndent, currentY), 0.7f * wordSize, glm::vec3(1.0f, 1.0f, 1.0f)));
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_f2_rotate", TextBox("  Numpad 8/2/4/6/7/9: Rotate", glm::vec2(rightX + subItemIndent, currentY), 0.7f * wordSize, glm::vec3(1.0f, 1.0f, 1.0f)));
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_f2_claw", TextBox("  Numpad +/-: Open/Close claw", glm::vec2(rightX + subItemIndent, currentY), 0.7f * wordSize, glm::vec3(1.0f, 1.0f, 1.0f)));
    currentY += sectionSpacing;
    
    // Other controls section
    textRenderer->addTextBox("inst_other", TextBox("Other Controls:", glm::vec2(rightX, currentY), 0.8f * wordSize, glm::vec3(0.9f, 0.9f, 0.3f)));
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_reset", TextBox("  R: Reset forcep positions", glm::vec2(rightX + subItemIndent, currentY), 0.7f * wordSize, glm::vec3(1.0f, 1.0f, 1.0f)));
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_help", TextBox("  F1: Toggle this help", glm::vec2(rightX + subItemIndent, currentY), 0.7f * wordSize, glm::vec3(1.0f, 1.0f, 1.0f)));
    currentY += lineSpacing;
    textRenderer->addTextBox("inst_exit", TextBox("  ESC: Exit program", glm::vec2(rightX + subItemIndent, currentY), 0.7f * wordSize, glm::vec3(1.0f, 1.0f, 1.0f)));
    
    // Initially hide instructions
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
    // Update forcep 1 status
    textRenderer->updateTextBox("forcep1_pos", "Position: " + formatVec3(forcep1Position));
    textRenderer->updateTextBox("forcep1_rot", "Rotation: " + formatVec3(forcep1Rotation));
    textRenderer->updateTextBox("forcep1_claw", "Claw Angle: " + formatFloat(forcep1ClawAngle) + " deg");
    
    // Update forcep 2 status
    textRenderer->updateTextBox("forcep2_pos", "Position: " + formatVec3(forcep2Position));
    textRenderer->updateTextBox("forcep2_rot", "Rotation: " + formatVec3(forcep2Rotation));
    textRenderer->updateTextBox("forcep2_claw", "Claw Angle: " + formatFloat(forcep2ClawAngle) + " deg");
    
    // Update mode
    textRenderer->updateTextBox("mode", "Mode: " + currentMode);
    
    // Update FPS
    textRenderer->updateTextBox("fps", "FPS: " + formatFloat(frameRate, 0));
    
    // Update collision status
    std::string collisionText = isColliding ? "Collision Status: Colliding" : "Collision Status: None";
    glm::vec3 collisionColor = isColliding ? glm::vec3(1.0f, 0.3f, 0.3f) : glm::vec3(0.5f, 1.0f, 0.5f);
    textRenderer->updateTextBox("collision", collisionText);
    textRenderer->updateTextBoxColor("collision", collisionColor);
}

void UIManager::updateMessage(float deltaTime) {
    if (messageTimer > 0.0f) {
        messageTimer -= deltaTime;
        if (messageTimer <= 0.0f) {
            hideMessage();
        } else {
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
    } else if (forcepId == 2) {
        forcep2Position = position;
    }
}

void UIManager::updateForcepRotation(int forcepId, const glm::vec3& rotation) {
    if (forcepId == 1) {
        forcep1Rotation = rotation;
    } else if (forcepId == 2) {
        forcep2Rotation = rotation;
    }
}

void UIManager::updateClawAngle(int forcepId, float angle) {
    if (forcepId == 1) {
        forcep1ClawAngle = angle;
    } else if (forcepId == 2) {
        forcep2ClawAngle = angle;
    }
}

void UIManager::updateMode(const std::string& mode) {
    currentMode = mode;
}

void UIManager::updateCollisionStatus(bool colliding) {
    isColliding = colliding;
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
    float centerX = (screenWidth - textSize.x) / 2.0f-30;
    float centerY = (screenHeight - textSize.y) / 2.0f;
    
    textRenderer->updateTextBox("message", message);
    textRenderer->updateTextBoxPosition("message", glm::vec2(centerX, centerY*0.8));
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
	float centerY = (screenHeight - textSize.y)/ 2.0f; // Center vertically

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
