#pragma once
#include "Model_new.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <tool.h>
#include "HapticDevice.h"


class Forcep {
public:
    // 使用智能指針管理模型組件
    std::shared_ptr<Model> body;
    std::shared_ptr<Model> clawUpper;
    std::shared_ptr<Model> clawLower;

    HapticDevice*  haptic = nullptr;

    bool hit = false;
    bool hitDeep = false;
    
	hduVector3Dd force = hduVector3Dd(0.0f, 0.0f, 0.0f);
    // 鉗子開合角度控制
    float clawAngle = 0.0f;
    float targetClawAngle = 20.0f;
    float clawSpeed = 30.0f; // 每秒最大變化角度
    glm::vec3 deltaPos = glm::vec3(0.0f);
    glm::vec3 deltaRot = glm::vec3(0.0f);
    glm::vec3 preRot = glm::vec3(0.0f);



    glm::vec3 OPos = glm::vec3(0.0f);
    glm::vec3 ORot = glm::vec3(0.0f);

    // 鉗爪相對於body的本地偏移位置
    glm::vec3 clawUpperOffset = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 clawLowerOffset = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 clawUpperPeak = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 clawLowerPeak = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 clawHitpointU = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 clawHitpointL = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 prePos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 prePosReal = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 penetrateMask = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 speed = glm::vec3(0.0f, 0.0f, 0.0f);
	btSphereOBJ* objB = nullptr; // 用於碰撞檢測的球形物件
	btCapsuleOBJ* objU = nullptr; // 上鉗爪的膠囊形物件
	btCapsuleOBJ* objL = nullptr; // 下鉗爪的膠囊形物件

public:
    // 建構函數 - 使用新的Model架構
    Forcep(const glm::vec3& position = glm::vec3(0.0f),
        const glm::vec3& rotation = glm::vec3(0.0f),
        float scale = 0.03f) {
		OPos = position;
		ORot = rotation;
		prePos = position;
        prePosReal = position;

        // 創建主體模型
        body = std::make_shared<Model>("./src/model/forceps/myforceps", scale, position, rotation);

        // 創建鉗爪模型
        clawUpper = std::make_shared<Model>("./src/model/forceps/myforceps_head1", 1.0f);
        clawLower = std::make_shared<Model>("./src/model/forceps/myforceps_head2", 1.0f);

        haptic = new HapticDevice;

        // 建立層次結構：鉗爪作為主體的子物件
        body->addChild(clawUpper);
        body->addChild(clawLower);

        // 設置鉗爪的本地位置
        clawUpper->setLocalPosition(clawUpperOffset);
        clawLower->setLocalPosition(clawLowerOffset);
        reset();
    }

    // === 位置和旋轉控制 ===

    // 設置整個鉗子的世界位置
    void setWorldPosition(const glm::vec3& position) {
        body->setWorldPosition(position);
    }

    // 設置整個鉗子的本地位置
    void setLocalPosition(const glm::vec3& position) {
        body->setLocalPosition(position);
    }

	void setRelatedWorldPosition(const glm::vec3& position) {
		body->setRelatedWorldPosition(position);
	}

    // 設置整個鉗子的本地旋轉
    void setLocalRotation(const glm::vec3& rotation) {
        body->setLocalRotation(rotation);
    }

    void setRelatedLocalRotation(const glm::vec3& rotation) {
        body->setRelatedLocalRotation(rotation);
    }

    // 設置整個鉗子的縮放
    void setLocalScale(float scale) {
        body->setLocalScale(scale);
    }

    // 獲取鉗子的世界位置
    glm::vec3 getWorldPosition() const {
        return body->getWorldPosition();
    }

    // 獲取鉗子的本地位置
    glm::vec3 getLocalPosition() const {
        return body->getLocalPosition();
    }

    // 獲取鉗子的本地旋轉
    glm::vec3 getLocalRotation() const {
        return body->getLocalRotation();
    }

    // === 變換操作 ===

    // 移動操作
    void translate(const glm::vec3& delta) {
        body->translate(delta);
    }

    // 旋轉操作
    void rotate(const glm::vec3& deltaAngle) {
        body->rotate(deltaAngle);
    }

    // 沿本地坐標軸移動
    void moveForward(float distance) {
        body->moveForward(distance);
    }

    void moveRight(float distance) {
        body->moveRight(distance);
    }

    void moveUp(float distance) {
        body->moveUp(distance);
    }

    // === 鉗爪控制 ===

    // 設置目標鉗爪角度
    void setClawAngle(float angle) {
        targetClawAngle = angle;
    }

    // 獲取當前鉗爪角度
    float getClawAngle() const {
        return clawAngle;
    }

    // 設置鉗爪動畫速度
    void setClawSpeed(float speed) {
        clawSpeed = speed;
    }

    // 立即設置鉗爪角度（無動畫）
    void setClawAngleImmediate(float angle) {
        clawAngle = angle;
        targetClawAngle = angle;
        updateClawPositions();
    }

    // 檢查鉗爪是否到達目標角度
    bool isClawAtTarget() const {
        return std::abs(clawAngle - targetClawAngle) < 0.1f;
    }

    // === 鉗爪偏移設置 ===

    // 設置上鉗爪的偏移位置
    void setClawUpperOffset(const glm::vec3& offset) {
        clawUpperOffset = offset;
        clawUpper->setLocalPosition(offset);
    }

    // 設置下鉗爪的偏移位置
    void setClawLowerOffset(const glm::vec3& offset) {
        clawLowerOffset = offset;
        clawLower->setLocalPosition(offset);
    }

    // === 層次結構操作 ===

    // 設置父物件
    void setParent(Model* parent) {
        body->setParent(parent);
    }

    // 添加為其他物件的子物件
    void addToParent(std::shared_ptr<Model> parent) {
        parent->addChild(body);
    }

    // === 更新和渲染 ===
    // 渲染鉗子（會自動渲染所有子組件）
    void Draw(Shader& shader) {
        body->Draw(shader);  // 會自動渲染所有子物件
    }

    // === Bullet Physics 整合 ===

    // 獲取用於物理引擎的變換數據
    void getBulletTransform(float* origin, float* rotation) const {
        body->getBulletTransform(origin, rotation);
    }

    // 從物理引擎更新變換
    void updateFromBullet(const float* origin, const float* rotation) {
        body->updateFromBullet(origin, rotation);
    }

    // === 便利方法 ===

    // 獲取鉗子的前方向量
    glm::vec3 getForwardVector() const {
        return body->getWorldAxisZ() * -1.0f;  // OpenGL中-Z是前方
    }

    // 獲取鉗子的右方向量
    glm::vec3 getRightVector() const {
        return body->getWorldAxisX();
    }

    // 獲取鉗子的上方向量
    glm::vec3 getUpVector() const {
        return body->getWorldAxisY();
    }

    // 重置鉗子到初始狀態
    void reset() {
		setWorldPosition(OPos);
		setLocalRotation(ORot);
        updateClawPositions();
    }

    // 更新鉗子狀態
    void updateClaw(float deltaTime) {
        // 更新鉗爪角度動畫
        if (clawAngle < targetClawAngle && !hit) {
            clawAngle += clawSpeed * deltaTime;
            if (clawAngle > targetClawAngle) {
                clawAngle = targetClawAngle;
            }
        }
        else if (clawAngle > targetClawAngle && !hit) {
            clawAngle -= clawSpeed * deltaTime;
            if (clawAngle < targetClawAngle) {
                clawAngle = targetClawAngle;
            }
        }

        // 更新鉗爪位置
        updateClawPositions(); 
		glm::vec3 worldPos = getWorldPosition();
        clawUpperPeak = worldPos + clawUpper->getWorldAxisX() * 1.0f;
        clawLowerPeak = worldPos + clawLower->getWorldAxisX() * 1.0f;
		//std::cout << "clawUpperPeak IN: " << clawUpperPeak.x << ", " << clawUpperPeak.y << ", " << clawUpperPeak.z << std::endl;
		clawHitpointU = (clawUpperPeak + worldPos) / 2.0f;
		clawHitpointL = (clawLowerPeak + worldPos) / 2.0f;
    }

    void updateMesh(btCollisionWorld* collisionWorld) {
        clawLower->updateMesh(collisionWorld);
        clawUpper->updateMesh(collisionWorld);
        if (!objB) {
            objB = new btSphereOBJ(getWorldPosition(), 0.3f);
        }
        if (!objU) {
            objU = new btCapsuleOBJ(getWorldPosition(), clawUpperPeak, 0.02f);
        }
        if (!objL) {
            objL = new btCapsuleOBJ(getWorldPosition(), clawLowerPeak, 0.02f);
        }
        objB->update(getWorldPosition());
        objU->update(getWorldPosition(), clawUpperPeak);
        objL->update(getWorldPosition(), clawLowerPeak);
    }

	void updateRealMesh(btCollisionWorld* collisionWorld) {
		clawLower->updateMesh(collisionWorld);
		clawUpper->updateMesh(collisionWorld);
        clawLower->bulletCollisionObject->setWorldTransform(btTransform(btQuaternion(0, 0, 0, 1), btVector3(clawLower->getWorldPosition().x, clawLower->getWorldPosition().y, clawLower->getWorldPosition().z)));
		clawUpper->bulletCollisionObject->setWorldTransform(btTransform(btQuaternion(0, 0, 0, 1), btVector3(clawUpper->getWorldPosition().x, clawUpper->getWorldPosition().y, clawUpper->getWorldPosition().z)));
		if (!objB) {
			objB = new btSphereOBJ(getWorldPosition(), 0.3f);
		}
		objB->update(getWorldPosition());
	}

    void updateCollision() {
        if (hit) {
			haptic->setForce(force); 
        }
        else {
            haptic->setForce(hduVector3Dd(0.0f, 0.0f, 0.0f)); // 如果沒有碰撞，則不施加力
            if (!hitDeep) {
				penetrateMask = prePos-getWorldPosition();
                prePos = getWorldPosition();
            }
        }
		speed = getWorldPosition() - prePosReal; // 計算速度
        prePosReal = getWorldPosition();
        
    }

    void updateHaptic(float deltaTime) {
        haptic->update();
        HDint button1 = haptic->getButton1State();
		HDint button2 = haptic->getButton2State();
        double *euler = haptic->getGIMBALAngle();
        glm::vec3 rotate(glm::degrees(euler[2]), 0.0f, 0.0f);
        if (button2) {
            hduVector3Dd delta = haptic->getDeltaPosition();
            deltaPos = glm::vec3(delta[0], delta[1], delta[2]) * 0.02f; // mm->m
            setRelatedWorldPosition(deltaPos);
            
            if (hit) {
                preRot = rotate; // 如果碰撞，則不旋轉
				deltaRot = glm::vec3(0.0f, 0.0f, 0.0f); // 不計算旋轉增量
			}
            else {
				deltaRot = rotate - preRot; // 計算旋轉增量
                preRot = rotate;
            }
            //std::cout << "Forcep1 Rotate: " << rotate.x << ", " << rotate.y << ", " << rotate.z << std::endl;
            setRelatedLocalRotation(deltaRot);
        }
        else {
            preRot = rotate;
        }
        if (button1) {
            setClawAngle(0.0f); // 關閉
        }
        else {
            setClawAngle(30.0f); // 打開
            haptic->resetDeltaBase();
        }
       updateClaw(deltaTime);
    }

private:
    // 更新鉗爪位置和旋轉
    void updateClawPositions() {
        if (clawUpper && clawLower) {
            // 上鉗爪旋轉
            glm::vec3 upperRotation = glm::vec3(0, clawAngle, 0);
            clawUpper->setLocalPosition(clawUpperOffset);
            clawUpper->setLocalRotation(upperRotation);

            // 下鉗爪旋轉（相反方向）
            glm::vec3 lowerRotation = glm::vec3(0, -clawAngle, 0);
            clawLower->setLocalPosition(clawLowerOffset);
            clawLower->setLocalRotation(lowerRotation);
        }
    }
};