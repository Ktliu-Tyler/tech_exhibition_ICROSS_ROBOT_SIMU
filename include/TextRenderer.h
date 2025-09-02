#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>
#include <string>
#include "shader.h"

struct Character {
    unsigned int textureID;  // 字元紋理的ID
    glm::ivec2   size;       // 字元大小
    glm::ivec2   bearing;    // 字元相對於基線的偏移
    unsigned int advance;    // 移動到下一個字元的水平距離
};

struct TextBox {
    std::string text;
    glm::vec2 position;     // 螢幕座標 (0,0)在左上角
    float scale;
    glm::vec3 color;
    bool visible;
    
    TextBox() : text(""), position(0.0f), scale(1.0f), color(1.0f), visible(true) {}
    TextBox(const std::string& txt, glm::vec2 pos, float s = 1.0f, glm::vec3 col = glm::vec3(1.0f))
        : text(txt), position(pos), scale(s), color(col), visible(true) {}
};

class TextRenderer {
public:
    TextRenderer(unsigned int screenWidth, unsigned int screenHeight);
    ~TextRenderer();
    
    bool initialize(const std::string& fontPath, unsigned int fontSize = 48);
    void renderText(const std::string& text, float x, float y, float scale, glm::vec3 color);
    void renderTextBox(const TextBox& textBox);
    
    // 文字方塊管理
    void addTextBox(const std::string& id, const TextBox& textBox);
    void updateTextBox(const std::string& id, const std::string& text);
    void updateTextBoxPosition(const std::string& id, glm::vec2 position);
    void updateTextBoxColor(const std::string& id, glm::vec3 color);
    void updateTextBoxScale(const std::string& id, float scale);
    void setTextBoxVisible(const std::string& id, bool visible);
    void removeTextBox(const std::string& id);
    void renderAllTextBoxes();
    
    // 工具函數
    glm::vec2 getTextSize(const std::string& text, float scale);
    void setScreenSize(unsigned int width, unsigned int height);

private:
    std::map<char, Character> characters;
    std::map<wchar_t, Character> wchar_characters;
    std::map<std::string, TextBox> textBoxes;
    unsigned int VAO, VBO;
    Shader* textShader;
    FT_Library ft;
    FT_Face face;
    unsigned int screenWidth, screenHeight;
    
    void setupOpenGL();
    void loadCharacters(unsigned int fontSize);
};

#endif
