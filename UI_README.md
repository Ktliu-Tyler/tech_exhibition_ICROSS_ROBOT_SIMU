# UI 文字渲染模組使用說明

## 功能特色

這個UI模組提供了以下功能：

1. **即時狀態顯示**
   - 夾子位置、旋轉角度、鉗爪角度
   - 當前操作模式（觸覺模式/鍵盤模式）
   - 碰撞檢測狀態
   - 實時FPS顯示

2. **互動式控制說明**
   - 按F1切換顯示/隱藏操作說明
   - 清楚的控制按鍵對應表

3. **訊息提示系統**
   - 可顯示臨時訊息（如模式切換、操作提示等）
   - 支援自定義顏色和顯示時間
   - 自動淡出效果

4. **響應式設計**
   - 自動適應視窗大小變化
   - 文字清晰易讀的字體渲染

## 使用方法

### 基本集成
```cpp
// 1. 創建UI管理器
std::unique_ptr<UIManager> uiManager = std::make_unique<UIManager>(screenWidth, screenHeight);

// 2. 初始化
if (!uiManager->initialize()) {
    // 處理初始化失敗
}

// 3. 在渲染循環中更新和渲染
while (!glfwWindowShouldClose(window)) {
    // ... 3D渲染代碼 ...
    
    // 更新UI狀態
    uiManager->updateForcepPosition(1, forcep1Position);
    uiManager->updateCollisionStatus(isColliding);
    
    // 更新UI邏輯
    uiManager->update(deltaTime);
    
    // 渲染UI (在所有3D渲染之後)
    uiManager->render();
}
```

### 顯示訊息
```cpp
// 顯示成功訊息（綠色，3秒）
uiManager->showMessage("操作成功！", 3.0f, glm::vec3(0.5f, 1.0f, 0.5f));

// 顯示警告訊息（黃色，2秒）
uiManager->showMessage("注意碰撞！", 2.0f, glm::vec3(1.0f, 1.0f, 0.0f));

// 顯示錯誤訊息（紅色，4秒）
uiManager->showMessage("操作失敗！", 4.0f, glm::vec3(1.0f, 0.3f, 0.3f));
```

### 控制說明顯示
```cpp
// 切換說明顯示
if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS) {
    uiManager->toggleInstructions();
}

// 直接設定說明可見性
uiManager->setInstructionsVisible(true);  // 顯示
uiManager->setInstructionsVisible(false); // 隱藏
```

## 按鍵控制

- **F1**: 切換操作說明顯示/隱藏
- **ESC**: 退出程序
- **其他按鍵**: 根據控制說明中的對應表操作

## 自定義擴展

如果需要添加更多UI元素，可以：

1. 在`UIManager::setupTextBoxes()`中添加新的文字框
2. 在`UIManager`類中添加對應的更新函數
3. 在主程式中調用這些更新函數

## 注意事項

1. 確保在OpenGL上下文創建後再初始化UI管理器
2. UI渲染應該在所有3D渲染完成後進行
3. 字體文件路徑需要正確設置（默認使用`src/fonts/arial.ttf`）
4. 支援視窗大小調整，會自動重新計算UI佈局
