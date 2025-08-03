#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "Model.h"
#include "shader.h"
#include "stb_easy_font.h"
#include "Forcep.h"

// 設定
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;
float SCALE_RATE = 1.0f;

// 相機
glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, 2.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 2.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));
glm::vec3 cmoveR(0.0f);
glm::vec3 cmoveL(0.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
float cameraRadius = 10.0f;


bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = SCR_WIDTH / 2.0;
float lastY = SCR_HEIGHT / 2.0;
float fov = 45.0f;

// 時間控制
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// 函數聲明
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

Model* wound = NULL;
Model* wound2 = NULL;
Shader* ourShader = NULL;
Forcep* myForcepL = NULL;
Forcep* myForcepR = NULL;

int main_old() {
    // 初始化 GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 創建窗口
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OBJ Model Loader", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // 告訴 GLFW 我們想要捕獲鼠標
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // 初始化 GLEW
    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // 配置全局 OpenGL 狀態
    glEnable(GL_DEPTH_TEST);

    // 建立和編譯著色器程序
    ourShader = new Shader("vertex_shader.glsl", "fragment_shader.glsl");

    // 設定 forcep 在鏡頭前方
    glm::vec3 forcepPosL = cameraPos + cameraFront * 3.0f - cameraRight * 1.0f;
    glm::mat4 forcepMatL = glm::translate(glm::mat4(1.0f), forcepPosL);
    glm::vec3 forcepPosR = cameraPos + cameraFront * 3.0f + cameraRight * 1.0f;
    glm::mat4 forcepMatR = glm::translate(glm::mat4(1.0f), forcepPosR);
	myForcepL = new Forcep(forcepMatL);
    myForcepR = new Forcep(forcepMatR);
    myForcepL->selfAngle = glm::vec3(0.0f, 0.0f, 300.0f);
    myForcepL->setModelMatrix();
    myForcepR->selfAngle = glm::vec3(0.0f, 0.0f, 240.0f);
    myForcepR->setModelMatrix();
   /* std::cout << myForcepL->body.scale_rate << std::endl;*/
	wound = new Model("./src/model/test/PDA_noPDA_3", 0.5f*SCALE_RATE, glm::vec3(0.0f, -5.0f, 0.0f), glm::vec3(-90.0f, 0.0f, 90.0f));
    wound2 = new Model("./src/model/test/PDA_with_aorta_no_PA_2_x0.05", 1.0f * SCALE_RATE, glm::vec3(0.0f, -2.0f, 0.0f), glm::vec3(180.0f, 0.0f, 0.0f));
    

    // 渲染循環
    while (!glfwWindowShouldClose(window)) {
        // 每幀時間邏輯
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 輸入
        processInput(window);
       
        // 渲染
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 啟用著色器
        ourShader->use();

        // 視圖/投影變換
        glm::mat4 projection = glm::perspective(glm::radians(fov),
            (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        ourShader->setMat4("projection", projection);
        ourShader->setMat4("view", view);
        ourShader->setVec3("lightPos", glm::vec3(5.0f, 5.0f, 5.0f)); // 你可以根據場景調整
        ourShader->setVec3("objectColor", glm::vec3(1.0f, 1.0f, 1.0f)); // 或根據模型材質
        ourShader->setVec3("lightColor", glm::vec3(2.0f, 1.0f, 1.0f));




  //      //讓 forcep 朝向與鏡頭一致
  //      glm::vec3 up = cameraUp;
  //      glm::vec3 front = glm::normalize(cameraFront);
  //      glm::vec3 right = glm::normalize(glm::cross(front, up));
  //      glm::mat4 rotMat = glm::mat4(glm::mat3(right, up, -front));
  //      forcepMat *= rotMat;


        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
            myForcepL->setClawAngle(0.0f);
        else
            myForcepL->setClawAngle(25.0f);
        myForcepL->update(deltaTime, *ourShader);
        myForcepL->Draw(*ourShader);
        

        if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
            myForcepR->setClawAngle(0.0f);
        else
            myForcepR->setClawAngle(25.0f);
        myForcepR->update(deltaTime, *ourShader);
        myForcepR->Draw(*ourShader);
        wound->Draw(*ourShader);
        wound2->Draw(*ourShader);
        if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
			std::cout << "ForcepL Position: " << "(" << myForcepL->worldPos.x << ", " << myForcepL->worldPos.y << ", " << myForcepL->worldPos.z << ")" << std::endl;
			std::cout << "Wound Position: " << "(" << wound->worldPos.x << ", " << wound->selfPos.y << ", " << wound->selfPos.z << ")" << std::endl;
        }




        // 交換緩衝區和輪詢 IO 事件
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理
    glfwTerminate();
    return 0;
}

// 處理輸入：查詢 GLFW 是否按下/釋放了相關按鍵並做出相應反應
void processInput(GLFWwindow* window) {
    float cameraSpeed = 2.5f * deltaTime;
    float moveSpeed = 3.0f * deltaTime; // 移動速度

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // 鏡頭移動（主座標系，i/j/k/l）
    glm::vec3 cmove(0.0f);
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
        cmove += cameraSpeed * glm::vec3(0, 1, 0); // cameraFront;
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        cmove -= cameraSpeed * glm::vec3(0, 1, 0); //cameraFront;
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        cmove -= cameraSpeed * glm::vec3(0, 0, 1); // cameraFront;
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        cmove += cameraSpeed * glm::vec3(0, 0, 1); //cameraFront;
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
        cmove -= cameraSpeed * glm::vec3(1, 0, 0); //glm::normalize(glm::cross(cameraFront, cameraUp)) ;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        cmove += cameraSpeed * glm::vec3(1, 0, 0); //glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	cameraPos += cmove;

    glm::vec3 forcepPosL = cameraPos + glm::vec3(-1, -3, -2); //cameraFront * 3.0f - cameraRight * 1.0f;
    myForcepL->posMat = glm::translate(glm::mat4(1.0f), forcepPosL);
    glm::vec3 forcepPosR = cameraPos + glm::vec3(1, -3, -2); //cameraFront * 3.0f + cameraRight * 1.0f;
    myForcepR->posMat = glm::translate(glm::mat4(1.0f), forcepPosR);

    // 左手（myForcepL）用WASD，世界座標
    if (myForcepL) {  
        glm::vec3 move(0.0f);
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            move += glm::vec3(0, 1, 0); // 世界Y+
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            move += glm::vec3(0, -1, 0); // 世界Y-
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            move += glm::vec3(-1, 0, 0); // 世界X-
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            move += glm::vec3(1, 0, 0); // 世界X+
        //move += cmove;
        if (glm::length(move) > 0.0f)
			cmoveL += glm::normalize(move) * moveSpeed;
        myForcepL->posMat = glm::translate(myForcepL->posMat, cmoveL);
    }
    

    // 右手（myForcepR）用↑↓←→，世界座標
    if (myForcepR) {
        glm::vec3 move(0.0f);
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            move += glm::vec3(0, 1, 0); // 世界Y+
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            move += glm::vec3(0, -1, 0); // 世界Y-
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            move += glm::vec3(-1, 0, 0); // 世界X-
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            move += glm::vec3(1, 0, 0); // 世界X+
        if (glm::length(move) > 0.0f)
			cmoveR += glm::normalize(move) * moveSpeed;
        myForcepR->posMat = glm::translate(myForcepR->posMat, cmoveR);
    }
}

// 當窗口大小改變時調用
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}


 //處理鼠標移動
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // 相反，因為 y 坐標從下到上
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f; // 改變這個值來調整鼠標敏感度
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // 確保當俯仰角超出範圍時，屏幕不會翻轉
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    pitch = -65;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

// 處理滾輪縮放
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    fov -= (float)yoffset;
    if (fov < 0.5f)
        fov = 0.5f;
    if (fov > 45.0f)
        fov = 45.0f;
}


