# iCROSS 手術力回饋模擬系統

以 C++、OpenGL 與 OpenHaptics 開發的手術互動展示程式，搭配 3D Systems 觸覺操作裝置，呈現新生兒開放性動脈導管（PDA）手術情境中的雙器械操作與接觸力回饋。

本專案的重點是串接觸覺輸入、三維模型、碰撞判定、回饋力計算與畫面顯示，讓操作者能在虛擬環境中移動器械、夾持血管，並感受接觸反作用力。程式用於實驗室研究與科技展展示；這份程式庫保留了開發期間的不同版本。

> **目前的重現狀態：** 本說明以 Visual Studio 專案所引用的 `main_new.cpp` 為主。儲存庫尚未包含執行所需的 OBJ 模型與第三方二進位函式庫，專案路徑及 CMake 來源清單也需要整理，因此目前不能僅以 clone 加上單一建置指令直接執行。以下列出實際程式流程與環境準備方式；尚未在乾淨環境中完成編譯及硬體驗證。

## 專案功能

- **手術場景顯示**：載入器械、傷口、血管與夾閉狀態模型，透過 OpenGL 繪製三維場景。
- **雙器械互動**：控制左右器械的位置、旋轉與鉗爪開合，包含鍵盤輸入與觸覺裝置輸入路徑。
- **碰撞與接觸回饋**：使用 Bullet 檢查器械與傷口、血管及另一支器械的接觸，取得接觸點與表面法向量等幾何資訊，再依各接觸情境計算回饋力。
- **觸覺裝置整合**：透過 OpenHaptics 讀取位置、萬向節角度及按鈕狀態，並輸出三軸力回饋。
- **展示介面**：顯示器械位置、姿態、鉗爪角度、碰撞狀態、回饋力與操作提示。
- **iCROSS 資料介面**：保留器械姿態轉換、機器人運動學及 UDP 指令傳送程式，供系統整合使用。

## 系統流程

```text
鍵盤／觸覺裝置輸入
        ↓
更新左右器械的位置、姿態與鉗爪
        ↓
更新模型與 Bullet 碰撞物件
        ↓
接觸判定 → 接觸點、表面方向與相對位置
        ↓
依接觸情境計算並彙整回饋力
        ↓
HapticDevice::setForce()
        ↓
OpenHaptics 非同步 callback → 裝置力輸出

OpenGL 與 UI 同時呈現場景及互動狀態
UDP 執行緒另行傳送器械對應的機器人指令
```

在 `main_new.cpp` 的主迴圈中，圖形、器械、碰撞與 UI 逐步更新；`HapticDevice` 則以 OpenHaptics 非同步 callback 讀取裝置並送出最新的力命令。**callback 的執行頻率與碰撞／力命令重新計算的頻率不同**，不能將畫面 FPS 或 SDK 的裝置更新率直接當作整套接觸模型的更新率。

`Forcep::updateCollision()` 會將累積的接觸力向量依 `forceCount` 取平均後送入裝置；未接觸時將力設為零。現有力模型主要依接觸幾何與情境參數構成，適合描述為互動展示的接觸力模型，尚無組織材料參數校準或臨床訓練成效的驗證資料。

## 程式導覽

| 檔案 | 主要內容 |
| --- | --- |
| [`main_new.cpp`](main_new.cpp) | 展示版主程式、場景建立、輸入、碰撞、接觸力、夾持流程及 UDP 執行緒 |
| [`include/HapticDevice.h`](include/HapticDevice.h) | OpenHaptics 初始化、裝置資料讀取及力輸出 callback |
| [`include/Forcep_new.h`](include/Forcep_new.h) | 器械與鉗爪狀態、觸覺位移映射及接觸力彙整 |
| [`include/Model_new.h`](include/Model_new.h) | Assimp 模型載入、座標轉換與 Bullet 三角網格碰撞物件 |
| [`include/Tool.h`](include/Tool.h) | 碰撞 callback、幾何及繪圖輔助工具 |
| [`src/UIManager_chi.cpp`](src/UIManager_chi.cpp) | Visual Studio 專案引用的中文展示介面 |
| [`src/TextRenderer.cpp`](src/TextRenderer.cpp) | FreeType 文字渲染 |
| [`src/iCROSS.cpp`](src/iCROSS.cpp) | iCROSS 機器人運動學計算 |
| [`src/Communication.cpp`](src/Communication.cpp) | Windows Winsock UDP 通訊 |
| [`include/Constant.h`](include/Constant.h) | 觸覺模式、單／雙裝置、視窗及場景設定 |
| [`src/definition.h`](src/definition.h) | UDP 目的位址、連接埠及其他共用定義 |
| [`project1.sln`](project1.sln)、[`project1.vcxproj`](project1.vcxproj) | Visual Studio 工程與目前來源檔案清單 |
| [`CMakeLists.txt`](CMakeLists.txt) | 另一套建置設定，需先整理來源清單與依賴 |

`main.cpp`、`main_modify.cpp`、`engine.cpp`、`engine_main.cpp` 與 `src/` 中的其他入口保留了早期或重構中的版本。不要將所有 `.cpp` 一起編譯。原有 [`ENGINE_README.md`](ENGINE_README.md) 描述另一套場景引擎設計，其中的選單與暫停操作不等同於 `main_new.cpp` 的實際操作方式。

## 開發環境與依賴

目前程式使用 Windows API 與 Winsock，建置設定以 **Windows x64、Visual Studio 2022／MSVC v143、C++17** 為整理基準。這不是已驗證的跨平台建置方案。

| 依賴 | 用途與現有設定 |
| --- | --- |
| OpenGL 3.3、GLFW、GLEW、GLM | 視窗、渲染及向量矩陣運算 |
| OpenHaptics SDK 與裝置驅動 | 3D Systems 觸覺裝置輸入與力輸出 |
| Bullet Physics | 三維碰撞及接觸查詢 |
| Assimp | OBJ 模型載入；工程目前引用 `assimp-vc140-mt.lib` |
| FreeType | 英文及中文文字繪製 |
| Eigen | 機器人運動學及矩陣計算 |
| OpenCV | 既有系統模組依賴；工程引用 `opencv_world3416.lib`／Debug 版本 |

儲存庫中的 `include/` 與 `lib/` 不能視為完整 SDK 安裝。需另外準備匹配 x64、編譯器、Debug／Release 及 runtime 設定的函式庫；目前工程同時列有 Debug 與 Release OpenCV 庫，應依所選組態調整。

## 重現與建置準備

1. 取得程式：

   ```powershell
   git clone https://github.com/Ktliu-Tyler/tech_exhibition_ICROSS_ROBOT_SIMU.git
   cd tech_exhibition_ICROSS_ROBOT_SIMU
   ```

2. 準備上述依賴，開啟 `project1.sln`，選擇 x64 組態。將工程中的原開發機絕對路徑改成目前電腦的 SDK、標頭與函式庫路徑，並檢查 C++17 與 UTF-8 編譯選項。

3. 補齊 `main_new.cpp` 與 `Forcep_new.h` 載入的模型，以及模型使用的材質和貼圖。主要 OBJ 路徑為：

   ```text
   src/model/forceps/myforceps.obj
   src/model/forceps/myforceps_head1.obj
   src/model/forceps/myforceps_head2.obj
   src/model/PDA_noPDA_3/PDA_noPDA_3.obj
   src/model/PDA_with_aorta/PDA_with_aorta_no_PA_2_long5.obj
   src/model/PDA_with_aorta/PDA_with_aorta_no_PA_2_clip.obj
   src/model/PDA_with_aorta/PDA_with_aorta_no_PA_2_squeeze.obj
   ```

   目前 Git 追蹤清單沒有上述 OBJ；`.gitignore` 亦有 `*.obj` 規則。取得模型後需另外保存資源，不能將模型 OBJ 與編譯器產生的 object 檔一併處理。

4. 以 `project1.vcxproj` 的來源清單為展示版起點，確認只保留一個有效的 `main()` 與一個 `UIManager` 實作。若改用 CMake，需先改為明確的來源清單，納入根目錄 `main_new.cpp`，排除 `src/main.cpp`、`src/engine_main.cpp` 及重複 UI 版本，並補上 Bullet 的標頭與連結設定。目前 CMake 的遞迴收集方式不能直接當作展示版的可靠建置入口。

5. 將程式的 **Working Directory 設為儲存庫根目錄**。程式從此處讀取 `vertex_shader.glsl`、`fragment_shader.glsl`、`src/fonts/msjh.ttf`、`src/shader/` 和 `src/model/`。只複製執行檔或 `model/` 資料夾不足以提供目前的相對路徑結構。

6. 檢查執行期 DLL 與 SDK 驅動後再建置。現階段仍需在具備原始資源及裝置的環境中完成編譯、啟動及操作驗證。

## 模式設定

在 [`include/Constant.h`](include/Constant.h) 中調整後需重新編譯：

| 設定 | 目前值 | 行為 |
| --- | --- | --- |
| `HAPTIC_MODE` | `false` | `false` 走鍵盤器械操作；`true` 初始化觸覺裝置 |
| `ONE_HAPTIC` | `false` | 觸覺模式下，`false` 使用左右兩台；`true` 僅初始化左側器械的裝置 |
| `FULL_SCREEN` | `true` | 全螢幕顯示 |
| `WIDTH`、`HEIGHT` | `1600`、`800` | 非全螢幕模式的視窗尺寸 |
| `WOUND_MOVE` | `true` | 啟用場景中的傷口／血管週期位移 |
| `SHOW_INSTRUCTION` | `false` | 初始操作說明顯示設定 |

雙裝置名稱為 `Left Device` 與 `Right Device`；單裝置路徑使用 `Default Device`。名稱需與裝置設定對應。`ONE_HAPTIC` 的第二支器械輸入流程仍應在實機上核對，不能視為已驗證的單裝置完整體驗。

鍵盤模式仍會編譯 OpenHaptics 相關程式並連結其依賴，並不表示可以省略 SDK。程式啟動時也會建立 UDP 傳送執行緒；目前 `src/definition.h` 的目的地為 `127.0.0.1:5005`。

## 操作方式

以下對應 `main_new.cpp`：

| 操作 | 按鍵 |
| --- | --- |
| 從選單開始體驗 | `Space` |
| 結束程式 | `Esc` |
| 顯示／隱藏操作說明 | 體驗中按 `F1` |
| 重置器械並清除成功狀態 | 體驗中按 `R` |
| 轉動視角／縮放 | 體驗中移動滑鼠／滾輪 |
| 左器械平移 | `W A S D`、`Q E` |
| 左器械旋轉／鉗爪 | `I K`、`J L`、`U O`／`N M` |
| 右器械平移 | 方向鍵、`PageUp PageDown` |
| 右器械旋轉／鉗爪 | 數字鍵盤 `8 2`、`4 6`、`7 9`／`+ -` |

觸覺模式下，裝置 **button 2** 用於啟用相對移動與力輸出，**button 1** 用於閉合鉗爪。按鈕編號依程式中的 OpenHaptics 定義；需配合實際裝置確認。

`T` 在目前入口只顯示切換提示，未實作完整的模式切換。`P` 暫停及數字選單屬於其他版本的文件內容，未列為此入口的功能。

## 研究重點與後續工作

開發中的核心問題包括接觸模型的連續性、器械與模型的座標對應、回饋力更新時序，以及視覺和觸覺的一致性。現有程式已建立上述模組之間的資料流程，後續仍可針對以下方向深化：

- 統一展示版入口、UI 版本及可重現的依賴與資源配置。
- 檢查主迴圈、UDP 執行緒與觸覺 callback 間共享資料的同步，量測各自的更新率與延遲。
- 針對接觸邊界、快速移動和多重接觸進行穩定性測試，評估力限制、阻尼與平滑策略。
- 加入模型參數、力輸出和操作軌跡記錄，建立可重複比較的測試方法。

目前未提供固定觸覺更新率、最大穩定剛性、延遲或臨床效益的量測結果。六自由度機構設計與馬達控制是相關研究工作，不能由本模擬程式直接推定已完成六自由度力／力矩輸出。

相關馬達控制工具：[SimplexMotion RS485／Modbus RTU Python 函式庫](https://github.com/Ktliu-Tyler/Simplexmotion-pymodbusRS485)。它是獨立專案，目前沒有在此展示程式中直接呼叫的整合路徑。

## 原始記錄與授權

原始 [`README.txt`](README.txt) 保留「未來科技展的程式」這段專案記錄。其他開發說明保留於 [`ENGINE_README.md`](ENGINE_README.md) 與 [`UI_README.md`](UI_README.md)，閱讀時請對照所使用的程式版本。

儲存庫目前未提供專案層級的 `LICENSE`。第三方 SDK、函式庫、字型與模型各自適用其授權條件，請勿將程式庫可存取視為所有素材均可任意再散布。
