#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <iomanip>
#include <winsock2.h>

#include "Communication.hpp"

// my lib
#include "shader.h"
#include "Model_new.h"
#include "Forcep_new.h"
#include "Tool.h"
#include "HapticDevice.h"
#include "UIManager.h"
#include "Constant.h"
#include "GameEngine.h"
#include "iCROSS.h"


