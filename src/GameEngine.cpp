#include "GameEngine.h"


GameEngine::GameEngine(unsigned int width, unsigned int height, const char* title)
	: screenWidth(width), screenHeight(height), windowTitle(title), currentState(GameState::MENU), nextState(GameState::MENU) {
	initialize();
}

GameEngine::~GameEngine() {
}

bool GameEngine::GameEngine::initialize() {
    uiManager = new UIManager(SCR_WIDTH, SCR_HEIGHT);
    if (!uiManager->initialize()) {
        std::cout << "Failed to initialize UI Manager" << std::endl;
        return -1;
    }

}

//void GameEngine::forcepKeyboard(GLFWwindow* window) {
//    // 夾子1控制 (WASD + QE + IJKL + UO + NM)
//    forcep1Control.moveForward = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
//    forcep1Control.moveBackward = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
//    forcep1Control.moveLeft = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
//    forcep1Control.moveRight = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
//    forcep1Control.moveUp = glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS;
//    forcep1Control.moveDown = glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS;
//
//    forcep1Control.rotateUp = glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS;
//    forcep1Control.rotateDown = glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS;
//    forcep1Control.rotateLeft = glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS;
//    forcep1Control.rotateRight = glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS;
//    forcep1Control.rollLeft = glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS;
//    forcep1Control.rollRight = glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS;
//
//    forcep1Control.openClaw = glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS;
//    forcep1Control.closeClaw = glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS;
//
//
//    // 夾子2控制 (方向鍵 + PageUp/PageDown + 數字鍵)
//    forcep2Control.moveForward = glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS;
//    forcep2Control.moveBackward = glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS;
//    forcep2Control.moveLeft = glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS;
//    forcep2Control.moveRight = glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS;
//    forcep2Control.moveUp = glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS;
//    forcep2Control.moveDown = glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS;
//    forcep2Control.rotateUp = glfwGetKey(window, GLFW_KEY_KP_8) == GLFW_PRESS;
//    forcep2Control.rotateDown = glfwGetKey(window, GLFW_KEY_KP_2) == GLFW_PRESS;
//    forcep2Control.rotateLeft = glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS;
//    forcep2Control.rotateRight = glfwGetKey(window, GLFW_KEY_KP_6) == GLFW_PRESS;
//    forcep2Control.rollLeft = glfwGetKey(window, GLFW_KEY_KP_7) == GLFW_PRESS;
//    forcep2Control.rollRight = glfwGetKey(window, GLFW_KEY_KP_9) == GLFW_PRESS;
//
//    forcep2Control.openClaw = glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS;
//    forcep2Control.closeClaw = glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS;
//}
//// 輸入處理
//void GameEngine::processInput(GLFWwindow* window) {
//
//    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//        glfwSetWindowShouldClose(window, true);
//
//    // F1 鍵切換說明顯示
//    if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS && !f1Pressed) {
//        uiManager->toggleInstructions();
//        f1Pressed = true;
//    }
//    if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_RELEASE) {
//        f1Pressed = false;
//    }
//
//    // 切換跟隨模式
//    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && !tPressed) {
//        // 這裡可以添加跟隨模式切換邏輯
//        uiManager->showMessage("Follow Mode Toggled", 2.0f, glm::vec3(0.5f, 1.0f, 0.5f));
//        tPressed = true;
//    }
//    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE) {
//        tPressed = false;
//    }
//
//    // 重置夾子位置
//    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !rPressed) {
//        // 重置邏輯可以在這裡添加
//        uiManager->showMessage("Forcep Position Reset", 2.0f, glm::vec3(1.0f, 0.5f, 0.5f));
//        rPressed = true;
//    }
//    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) {
//        rPressed = false;
//    }
//}
//void GameEngine::checkCollisionWithWound(btCollisionWorld& world, const std::vector<std::shared_ptr<Forcep>>& forceps, Model* wound) {
//
//    for (auto& f : forceps) {
//        if (!f->objU || !f->objL || f->hit) continue;
//        std::vector<btCapsuleOBJ*> objs = { f->objU, f->objL };
//        for (auto& obj : objs) {
//            MyResult2Callback result(obj->Trans);
//            world.contactPairTest(&(obj->Object), wound->bulletCollisionObject, result);
//            if (result.hit && !f->hit) {
//                f->hit = true;
//                //std::cout << "UPOS: " << f->getWorldPosition().x << ", " << f->getWorldPosition().y << ", " << f->getWorldPosition().z << std::endl;
//                //std::cout << "prePOS: " << f->prePos.x << ", " << f->prePos.y << ", " << f->prePos.z << std::endl;
//                glm::vec3 dir = f->prePos - f->getWorldPosition();
//                glm::vec3 hitP = glm::vec3(result.hitPoint.x(), result.hitPoint.y(), result.hitPoint.z());
//                //glm::vec3 dir = obj->C - hitP;
//                //glm::vec3 dir = f->clawHitpointU - f->getWorldPosition();
//                //std::cout << "hit Dir: " << dir.x << ", " << dir.y << ", " << dir.z << std::endl;
//                float dirALL = sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
//                float rate1 = 0.0f;
//                float rate2 = 0.0f;
//                float rate = 0.0f;
//
//
//                if (dirALL > 0.001f) {
//                    f->hitDeep = true;
//                }
//                else {
//                    f->hitDeep = false;
//                }
//
//                if (!f->hitDeep) {
//                    hduVector3Dd force(f->speed.x, f->speed.y, f->speed.z);
//                    f->force = force * (-1.0f); // 設定回饋力
//                    f->prePos += f->speed;
//                }
//                else {
//                    rate1 = dirALL / 0.02f;
//                    rate2 = 0.5f / dirALL;
//                    rate = rate1 + rate2;
//                    hduVector3Dd force(dir.x * rate, dir.y * rate, dir.z * rate);
//                    f->force = force * 1.0f;
//                }
//
//
//
//                //        else if (dirALL < 0.02f) {
//                            //rate1 = 0.0f; // 當距離小於 0.02 時，回饋力為 0
//                //        }
//
//                std::cout << "dirALL: " << dirALL << std::endl;
//                //std::cout << "rate: " << rate * dirALL << std::endl;
//
//                f->prePos += (wound->getWorldPosition() - wound->prePos);
//                glm::vec3 pushDir = wound->getWorldPosition() - wound->prePos;
//                glm::vec3 forcepToWound = f->getWorldPosition() - wound->prePos;
//
//                if (glm::dot(pushDir, forcepToWound) > 0) {
//                    f->setRelatedWorldPosition(pushDir);
//                    float rate = 500.0f;
//                    /*hduVector3Dd force(pushDir.x* rate, pushDir.y * rate, pushDir.z * rate);
//                    std::cout << "Push Dir: " << pushDir.x << ", " << pushDir.y << ", " << pushDir.z << std::endl;
//                    f->force += force;*/
//                }
//                //std::cout << "Force: " << f->force << std::endl;
//
//            }
//        }
//    }
//    wound->prePos = wound->getWorldPosition();
//
//}
//
////void GameEngine::checkCollisionWithWound_(btCollisionWorld& world, const std::vector<std::shared_ptr<Forcep>>& forceps, Model* wound) {
////
////    for (auto& f : forceps) {
////        if (!f->objU || !f->objL || f->hit) continue;
////        std::vector<btCapsuleOBJ*> objs = { f->objU, f->objL };
////        for (auto& obj : objs) {
////            MyResult2Callback result(obj->Trans);
////            world.contactPairTest(&(obj->Object), wound->bulletCollisionObject, result);
////            if (result.hit && !f->hit) {
////                f->hit = true;
////                //std::cout << "UPOS: " << f->getWorldPosition().x << ", " << f->getWorldPosition().y << ", " << f->getWorldPosition().z << std::endl;
////                //std::cout << "prePOS: " << f->prePos.x << ", " << f->prePos.y << ", " << f->prePos.z << std::endl;
////                glm::vec3 Odir = f->prePos - f->getWorldPosition();
////                glm::vec3 dir = dot(Odir, normalize(f->penetrateMask)) * normalize(f->penetrateMask);
////                glm::vec3 hitP = glm::vec3(result.hitPoint.x(), result.hitPoint.y(), result.hitPoint.z());
////                //glm::vec3 dir = obj->C - hitP;
////                //glm::vec3 dir = f->clawHitpointU - f->getWorldPosition();
////                //std::cout << "hit Dir: " << dir.x << ", " << dir.y << ", " << dir.z << std::endl;
////                float dirALL = sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
////                float rate1 = 0.0f;
////                float rate2 = 0.0f;
////                float rate = 0.0f;
////
////
////                if (dirALL > 0.001f) {
////                    f->hitDeep = true;
////                }
////                else {
////                    f->hitDeep = false;
////                }
////
////                if (!f->hitDeep) {
////                    hduVector3Dd force(f->speed.x, f->speed.y, f->speed.z);
////                    f->force = force * (-1.0f); // 設定回饋力
////                    f->prePos += f->speed;
////                }
////                else {
////                    rate1 = dirALL / 0.02f;
////                    rate2 = 0.5f / dirALL;
////                    rate = rate1 + rate2;
////                    hduVector3Dd force(dir.x * rate, dir.y * rate, dir.z * rate);
////                    f->force = force * 1.0f;
////                }
////
////
////
////                //        else if (dirALL < 0.02f) {
////                            //rate1 = 0.0f; // 當距離小於 0.02 時，回饋力為 0
////                //        }
////
////                std::cout << "dirALL: " << dirALL << std::endl;
////                //std::cout << "rate: " << rate * dirALL << std::endl;
////
////                f->prePos += (wound->getWorldPosition() - wound->prePos);
////                glm::vec3 pushDir = wound->getWorldPosition() - wound->prePos;
////                glm::vec3 forcepToWound = f->getWorldPosition() - wound->prePos;
////
////                if (glm::dot(pushDir, forcepToWound) > 0) {
////                    f->setRelatedWorldPosition(pushDir);
////                    float rate = 500.0f;
////                    /*hduVector3Dd force(pushDir.x* rate, pushDir.y * rate, pushDir.z * rate);
////                    std::cout << "Push Dir: " << pushDir.x << ", " << pushDir.y << ", " << pushDir.z << std::endl;
////                    f->force += force;*/
////                }
////                //std::cout << "Force: " << f->force << std::endl;
////
////            }
////        }
////    }
////    wound->prePos = wound->getWorldPosition();
////
////}
////
//
//void GameEngine::checkCollisionWithVessel(btCollisionWorld& world, const std::vector<std::shared_ptr<Forcep>>& forceps, Model* vessel) {
//
//    for (auto& f : forceps) {
//        if (!f->objU || !f->objL || f->hit) continue;
//        std::vector<btCapsuleOBJ*> objs = { f->objU, f->objL };
//        for (auto& obj : objs) {
//            MyResultCallback result(obj->Trans);
//            world.contactPairTest(&(obj->Object), vessel->bulletCollisionObject, result);
//            if (result.hit && !f->hit) {
//                f->hit = true;
//                glm::vec3 dir = f->prePos - f->getWorldPosition();
//                std::cout << "hit Dir: " << dir.x << ", " << dir.y << ", " << dir.z << std::endl;
//                float dirALL = sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
//                float rate1 = dirALL / 0.02f;
//                float rate2 = 0.5f / dirALL;
//                float rate = 0.0f;
//                if (dirALL == 0)
//                    rate1 = 0.0f; // 避免除以零
//                rate = rate1 + rate2;
//
//                hduVector3Dd force(dir.x * rate, dir.y * rate, dir.z * rate);
//                f->force = force * 1.0f; // 設定回饋力
//                f->prePos += (vessel->getWorldPosition() - vessel->prePos);
//                glm::vec3 pushDir = vessel->getWorldPosition() - vessel->prePos;
//                glm::vec3 forcepToVessel = f->getWorldPosition() - vessel->prePos;
//
//                if (glm::dot(pushDir, forcepToVessel) > 0) {
//                    f->setRelatedWorldPosition(pushDir);
//                    float rate = 500.0f;
//                    /*hduVector3Dd force(pushDir.x* rate, pushDir.y * rate, pushDir.z * rate);
//                    std::cout << "Push Dir: " << pushDir.x << ", " << pushDir.y << ", " << pushDir.z << std::endl;
//                    f->force += force;*/
//                }
//                std::cout << "Force: " << f->force << std::endl;
//
//            }
//        }
//    }
//    vessel->prePos = vessel->getWorldPosition();
//}

