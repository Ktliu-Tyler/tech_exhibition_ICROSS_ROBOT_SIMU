# 遊戲引擎使用說明

## 功能概述

這個遊戲引擎提供了完整的頁面切換系統，支援以下場景：

- **菜單場景 (MENU)**: 主菜單，可選擇開始遊戲、設定或退出
- **遊戲場景 (GAME)**: 主要的遊戲邏輯執行場景
- **暫停場景 (PAUSE)**: 遊戲暫停時的選項菜單
- **設定場景 (SETTINGS)**: 遊戲設定（目前重定向到菜單）

## 按鍵控制

### 菜單場景
- **數字鍵 1-3**: 直接選擇選項
- **UP/DOWN 方向鍵**: 上下選擇
- **ENTER**: 確認選擇

### 遊戲場景
- **ESC**: 回到主菜單
- **P**: 暫停遊戲
- **其他按鍵**: 你的遊戲邏輯按鍵

### 暫停場景
- **數字鍵 1-2**: 直接選擇選項
- **UP/DOWN 方向鍵**: 上下選擇
- **ENTER**: 確認選擇
- **P**: 直接繼續遊戲

## 使用方法

### 1. 基本使用

```cpp
#include "GameEngine.h"

int main() {
    // 創建引擎實例
    GameEngine engine(1200, 800, "My Game");
    
    // 初始化
    if (!engine.initialize()) {
        return -1;
    }
    
    // 運行主循環
    engine.run();
    
    return 0;
}
```

### 2. 整合現有遊戲邏輯

在 `GameScene` 類中整合你現有的遊戲代碼：

```cpp
// 在 GameScene::initialize() 中
void GameScene::initialize() {
    // 初始化你的遊戲對象
    auto forcep1 = std::make_shared<Forcep>(...);
    auto forcep2 = std::make_shared<Forcep>(...);
    // ... 其他初始化
}

// 在 GameScene::update() 中
void GameScene::update(float deltaTime) {
    // 你的遊戲更新邏輯
    forcep1->update(deltaTime);
    forcep2->update(deltaTime);
    // ... 其他更新
}

// 在 GameScene::render() 中
void GameScene::render() {
    // 你的3D渲染邏輯
    ourShader.use();
    forcep1->Draw(ourShader);
    forcep2->Draw(ourShader);
    // ... 其他渲染
}
```

### 3. 自定義場景

你可以創建新的場景類：

```cpp
class SettingsScene : public Scene {
public:
    void initialize() override {
        // 設定場景初始化
    }
    
    void update(float deltaTime) override {
        // 設定場景更新
    }
    
    void render() override {
        // 設定場景渲染
    }
    
    void processInput(GLFWwindow* window) override {
        // 設定場景輸入處理
    }
};
```

### 4. 狀態管理

在場景中改變狀態：

```cpp
void MyScene::processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        currentState = GameState::MENU; // 回到菜單
    }
}
```

## 架構說明

### 核心類別

1. **GameEngine**: 主要的引擎控制器
   - 管理GLFW視窗和OpenGL上下文
   - 控制場景切換
   - 處理主遊戲循環

2. **Scene**: 抽象場景基類
   - 定義場景的標準介面
   - 每個場景都需要實現初始化、更新、渲染和輸入處理

3. **MenuScene**: 菜單場景實現
4. **GameScene**: 遊戲場景實現
5. **PauseScene**: 暫停場景實現

### 狀態流程

```
MENU → GAME → PAUSE
  ↑      ↓       ↓
  ←──────────────←
```

## 擴展指南

### 添加新場景

1. 在 `GameState` 枚舉中添加新狀態
2. 創建新的場景類繼承 `Scene`
3. 在 `GameEngine::createScene()` 中添加創建邏輯
4. 在適當的場景中添加切換邏輯

### 添加新控制

在相應的 `processInput()` 方法中添加按鍵處理邏輯。

### 整合現有代碼

將你現有的 `main_new.cpp` 中的邏輯移動到 `GameScene` 類中：
- 全域變數移動到 `GameScene` 的私有成員
- 初始化代碼移動到 `initialize()`
- 遊戲循環邏輯移動到 `update()` 和 `render()`
- 輸入處理移動到 `processInput()`

## 編譯說明

確保你的項目包含以下文件：
- `include/GameEngine.h`
- `engine.cpp` (或重命名為 `src/GameEngine.cpp`)
- 現有的UI系統文件 (`TextRenderer.h/cpp`, `UIManager.h/cpp`)

在Visual Studio項目中添加這些文件，然後編譯運行。
