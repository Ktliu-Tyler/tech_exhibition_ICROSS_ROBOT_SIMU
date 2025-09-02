#include "TextRenderer.h"
#include <iostream>
#include <codecvt>
#include <locale>


TextRenderer::TextRenderer(unsigned int screenWidth, unsigned int screenHeight) 
    : screenWidth(screenWidth), screenHeight(screenHeight), textShader(nullptr) {
}

TextRenderer::~TextRenderer() {
    // 清理FreeType資源
    if (face) {
        FT_Done_Face(face);
    }
    if (ft) {
        FT_Done_FreeType(ft);
    }
    
    // 清理OpenGL資源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    
    if (textShader) {
        delete textShader;
    }
}

bool TextRenderer::initialize(const std::string& fontPath, unsigned int fontSize) {
    // 初始化FreeType
    if (FT_Init_FreeType(&ft)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return false;
    }

    // 載入字體
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font from " << fontPath << std::endl;
        return false;
    }

    // 設定字體大小
    FT_Set_Pixel_Sizes(face, 0, fontSize);

    // 禁用位元組對齊限制
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // 載入字元
    loadCharacters(fontSize);

    // 設定OpenGL
    setupOpenGL();

    // 創建著色器
    textShader = new Shader("src/shader/text_vertex.glsl", "src/shader/text_fragment.glsl");

    return true;
}

//void TextRenderer::loadCharacters(unsigned int fontSize) {
//    // 載入前128個ASCII字元
//    for (unsigned char c = 0; c < 128; c++) {
//        // 載入字元字形
//        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
//            std::cout << "ERROR::FREETYTPE: Failed to load Glyph " << c << std::endl;
//            continue;
//        }
//
//        // 生成紋理
//        unsigned int texture;
//        glGenTextures(1, &texture);
//        glBindTexture(GL_TEXTURE_2D, texture);
//        glTexImage2D(
//            GL_TEXTURE_2D,
//            0,
//            GL_RED,
//            face->glyph->bitmap.width,
//            face->glyph->bitmap.rows,
//            0,
//            GL_RED,
//            GL_UNSIGNED_BYTE,
//            face->glyph->bitmap.buffer
//        );
//
//        // 設定紋理選項
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//        // 儲存字元供後續使用
//        Character character = {
//            texture,
//            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
//            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
//            static_cast<unsigned int>(face->glyph->advance.x)
//        };
//        characters.insert(std::pair<char, Character>(c, character));
//    }
//    glBindTexture(GL_TEXTURE_2D, 0);
//}

void TextRenderer::loadCharacters(unsigned int fontSize) {
    // 載入ASCII
    for (unsigned char c = 0; c < 128; c++) {
        // 載入字元字形
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph " << c << std::endl;
            continue;
        }
        
        // 生成紋理
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        
        // 設定紋理選項
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // 儲存字元供後續使用
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)
        };
        wchar_characters.insert(std::pair<char, Character>(c, character));
    }
    // 載入常用中文字
    for (wchar_t c = 0x4E00; c <= 0x9FFF; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            continue;
        }
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)
        };
        // 注意：key型態要改成wchar_t
        wchar_characters.insert(std::pair<wchar_t, Character>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}


void TextRenderer::setupOpenGL() {
    // 配置VAO/VBO來渲染紋理四邊形
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void TextRenderer::renderText(const std::string& text, float x, float y, float scale, glm::vec3 color) {
    
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    std::wstring wtext = conv.from_bytes(text);
    // 啟用混合以支援透明度
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 啟用著色器
    textShader->use();
    
    // 設定投影矩陣 (正交投影)
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(screenWidth), 0.0f, static_cast<float>(screenHeight));
    textShader->setMat4("projection", projection);
    textShader->setVec3("textColor", color.x, color.y, color.z);
    
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // 遍歷所有字元
    std::string::const_iterator c;
    for (wchar_t c : wtext) {
        Character ch = wchar_characters[c];

        float xpos = x + ch.bearing.x * scale;
        float ypos = y - (ch.size.y - ch.bearing.y) * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;

        // 為每個字元更新VBO
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };

        // 在四邊形上渲染字形紋理
        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        // 更新VBO記憶體的內容
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // 渲染四邊形
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // 前進游標為下一個字形
        x += (ch.advance >> 6) * scale; // 位移6位來獲得像素值 (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
}

void TextRenderer::renderTextBox(const TextBox& textBox) {
    if (textBox.visible) {
        // 轉換座標系統：從螢幕座標(左上角原點)轉換到OpenGL座標(左下角原點)
        float openglY = screenHeight - textBox.position.y;
        renderText(textBox.text, textBox.position.x, openglY, textBox.scale, textBox.color);
    }
}

void TextRenderer::addTextBox(const std::string& id, const TextBox& textBox) {
    textBoxes[id] = textBox;
}

void TextRenderer::updateTextBox(const std::string& id, const std::string& text) {
    auto it = textBoxes.find(id);
    if (it != textBoxes.end()) {
        it->second.text = text;
    }
}

void TextRenderer::updateTextBoxPosition(const std::string& id, glm::vec2 position) {
    auto it = textBoxes.find(id);
    if (it != textBoxes.end()) {
        it->second.position = position;
    }
}

void TextRenderer::updateTextBoxColor(const std::string& id, glm::vec3 color) {
    auto it = textBoxes.find(id);
    if (it != textBoxes.end()) {
        it->second.color = color;
    }
}

void TextRenderer::updateTextBoxScale(const std::string& id, float scale) {
    auto it = textBoxes.find(id);
    if (it != textBoxes.end()) {
        it->second.scale = scale;
    }
}

void TextRenderer::setTextBoxVisible(const std::string& id, bool visible) {
    auto it = textBoxes.find(id);
    if (it != textBoxes.end()) {
        it->second.visible = visible;
    }
}

void TextRenderer::removeTextBox(const std::string& id) {
    textBoxes.erase(id);
}

void TextRenderer::renderAllTextBoxes() {
    for (const auto& pair : textBoxes) {
        renderTextBox(pair.second);
    }
}

glm::vec2 TextRenderer::getTextSize(const std::string& text, float scale) {
    float width = 0.0f;
    float height = 0.0f;
    
    for (char c : text) {
        Character ch = characters[c];
        width += (ch.advance >> 6) * scale;
        float charHeight = ch.size.y * scale;
        if (charHeight > height) {
            height = charHeight;
        }
    }
    
    return glm::vec2(width, height);
}

void TextRenderer::setScreenSize(unsigned int width, unsigned int height) {
    screenWidth = width;
    screenHeight = height;
}
