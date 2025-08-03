#pragma once
#include "Model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Forcep {
public:
    Model body;
    Model clawUpper;
    Model clawLower;

    // 夾爪本體的 model matrix
    glm::mat4 bodyMat = glm::mat4(1.0f);
    glm::mat4 rotMat = glm::mat4(1.0f);
    glm::mat4 posMat = glm::mat4(1.0f);
    glm::mat4 model = glm::mat4(0.0f);

	glm::vec3 worldPos = glm::vec3(0.0f); // 世界座標系下的姿態矩陣
    glm::vec3 selfPos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 selfAngle = glm::vec3(0.0f, 0.0f, 0.0f);

    // 夾爪開合角度（度）
    float clawAngle = 0.0f;
    float targetClawAngle = 20.0f;
    float clawSpeed = 30.0f; // 每秒最大變化角度
	float scale_rate = 1.0f; // 縮放比例
    

    // 爪片相對於body的本地平移（根據你的模型原點調整）
    glm::vec3 clawUpperOffset = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 clawLowerOffset = glm::vec3(0.0f, 0.0f, 0.0f);

    Forcep(glm::mat4 mat): body("./src/model/forceps/myforceps", 0.03f),
        clawUpper("./src/model/forceps/myforceps_head1", 0.03f),
        clawLower("./src/model/forceps/myforceps_head2", 0.03f)
    {
		posMat = mat;

    }

    void setBodyMatrix(const glm::mat4& posmat, const glm::mat4& rotmat) {
		posMat = posmat;
		rotMat = rotmat;
        bodyMat = posmat * rotmat;
		glm::vec3 pos = glm::vec3(posMat[3]); // 提取平移部分;
		worldPos = pos; // 更新世界座標系下的姿態矩陣
    }

    void setModelMatrix() {
        model = glm::mat4(1.0f);
        model = glm::translate(model, selfPos); 
        model = glm::scale(model, glm::vec3(scale_rate, scale_rate, scale_rate));     
        model = glm::rotate(model, glm::radians(selfAngle.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(selfAngle.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(selfAngle.z), glm::vec3(0.0f, 0.0f, 1.0f));
    }

    // 設定夾爪開合角度（度）
    void setClawAngle(float angle) {
        targetClawAngle = angle;
    }

    void update(float deltaTime, Shader& shader) {
        //Draw(shader);

		// 更新夾爪角度
        if (clawAngle < targetClawAngle) {
            clawAngle += clawSpeed * deltaTime;
            if (clawAngle > targetClawAngle) clawAngle = targetClawAngle;
        }
        else if (clawAngle > targetClawAngle) {
            clawAngle -= clawSpeed * deltaTime;
            if (clawAngle < targetClawAngle) clawAngle = targetClawAngle;
        }
    }

    // 一起繪製
    void Draw(Shader& shader) {
        // 畫body
        bodyMat = posMat * rotMat; 
        body.setMatrix(modeling(bodyMat));
		body.Draw(shader);

        // 爪片的相對矩陣（以body為基準，先平移再旋轉）
        glm::mat4 upperMat = modeling(bodyMat);
        upperMat = glm::translate(upperMat, clawUpperOffset);
        upperMat = glm::rotate(upperMat, glm::radians(clawAngle), glm::vec3(0, 1, 0)); // 依需求調整旋轉軸
        clawUpper.setMatrix(upperMat);
        clawUpper.Draw(shader);

        glm::mat4 lowerMat = modeling(bodyMat);
        lowerMat = glm::translate(lowerMat, clawLowerOffset);
        lowerMat = glm::rotate(lowerMat, glm::radians(-clawAngle), glm::vec3(0, 1, 0)); // 依需求調整旋轉軸
        clawLower.setMatrix(lowerMat);
		clawLower.Draw(shader);  
    }
private:
    glm::mat4 modeling(glm::mat4 mat) {
		return mat * model; 
    }
};
#pragma once
