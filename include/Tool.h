#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <vector>

enum class GameState {
    MENU,
    GAME,
    GAME_SUCCESS,
    PAUSE,
    SETTINGS,
    EXIT
};

struct MyResultCallback : public btCollisionWorld::ContactResultCallback {
    bool hit = false;
    btVector3 hitPoint;
    btVector3 hitNormal;
    btScalar minDist = FLT_MAX;  // �אּ�̤j��
    btTransform testTrans;

    MyResultCallback(const btTransform& t) : testTrans(t) {}

    btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0,
        const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1) override {
        hit = true;

        
        if (cp.getDistance() < minDist) {
            minDist = cp.getDistance();
            hitPoint = cp.getPositionWorldOnB();
            hitNormal = cp.m_normalWorldOnB;
        }

        return 0;
    }
};

class SurfaceNormalCallback : public btCollisionWorld::ContactResultCallback {
public:
    btTransform trans;
    bool hit = false;
    btVector3 hitPoint;
	glm::vec3 hitPointGLM;
    btVector3 surfaceNormal; 
    btScalar minDistance = 0.05f;
    btScalar penetrationDepth = 0.0f;

    SurfaceNormalCallback(btTransform t) : trans(t), hit(false) {}

    btScalar addSingleResult(btManifoldPoint& cp,
        const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0,
        const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1) override {

        if (cp.getDistance() < minDistance) {
            minDistance = cp.getDistance();
            hit = true;
            hitPoint = cp.getPositionWorldOnA();
			hitPointGLM = glm::vec3(hitPoint.x(), hitPoint.y(), hitPoint.z());
            surfaceNormal = cp.m_normalWorldOnB;
            penetrationDepth = -cp.getDistance();
            if (cp.getDistance() > 0) {
                penetrationDepth = 0.0f;
            }
        }
        return 0;
    }
    void reset() {
        hit = false;
        minDistance = FLT_MAX;
        penetrationDepth = 0.0f;
        hitPoint = btVector3(0, 0, 0);
        surfaceNormal = btVector3(0, 1, 0);
    }
};

class MyResult2Callback : public btCollisionWorld::ContactResultCallback {
public:
    btTransform trans;
    bool hit = false;
    btVector3 hitPoint;
    btVector3 normal;  

    MyResult2Callback(btTransform t) : trans(t), hit(false) {}

    btScalar addSingleResult(btManifoldPoint& cp,
        const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0,
        const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1) override {
        hit = true;
        hitPoint = cp.getPositionWorldOnA();  
        normal = cp.m_normalWorldOnB;         
        return 0;
    }
};

class btSphereOBJ {
public:
    btVector3 Point;
    btTransform Trans;
    btSphereShape *Shape;
    btCollisionObject Object;

    btSphereOBJ(const glm::vec3& pos, float radius)
        : Point(pos.x, pos.y, pos.z)
    {
        Trans.setIdentity();
        Trans.setOrigin(Point);
		Shape = new btSphereShape(radius);
        Object.setCollisionShape(Shape);
        Object.setWorldTransform(Trans);
    }
    void update(const glm::vec3& pos) {
        Point = btVector3(pos.x, pos.y, pos.z);
        Trans.setOrigin(Point);
		Object.setWorldTransform(Trans);
    }
};

class btCapsuleOBJ {
public:
    btVector3 Point;
    btTransform Trans;
    btCapsuleShape * Shape;
    btCollisionObject Object;
    btDefaultMotionState* MotionState;
    btRigidBody* Body;

    float radius = 1.0f;
    float length = 1.0f;
    float mass = 1.0f;
    bool hit = false;
    glm::vec3 hitpoint;
	float hitdepth = -1.0f;
	glm::vec3 arr[20] = { glm::vec3(0)};
    glm::vec3 center;
    glm::vec3 dir;
	glm::vec3 hitdir = glm::vec3(0);
    glm::vec3 A;
    glm::vec3 B;
    glm::vec3 C;
    glm::vec3 up;
   

    btCapsuleOBJ(const glm::vec3& a, const glm::vec3& b, float r = 1.0f)
        : A(a), B(b), radius(r) 
    {
        A = a;
        B = b;
		C = (A + B) / 2.0f;
        for (int i = 0; i < 20; ++i) {
            float t = static_cast<float>(i) / 19.0f; // 0~1, 共20個點
            arr[i] = (1.0f - t) * A + t * B;
        }
        center = (a + b) * 0.5f;
        dir = glm::normalize(b - a);
        up = glm::vec3(0, 1, 0);
        length = glm::distance(A, B);
        glm::quat q = rotate();
        Trans.setIdentity();
        Trans.setOrigin(btVector3(center.x, center.y, center.z));
        Trans.setRotation(btQuaternion(q.x, q.y, q.z, q.w));
        Shape = new btCapsuleShape(radius, length - 2 * radius);
        Object.setCollisionShape(Shape);
        Object.setWorldTransform(Trans);

        MotionState = new btDefaultMotionState(Trans);
        btVector3 inertia(0, 0, 0);
        if (mass > 0.0f)
            Shape->calculateLocalInertia(mass, inertia);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, MotionState, Shape, inertia);
        Body = new btRigidBody(rbInfo);

        

    }


    void update(const glm::vec3& a, const glm::vec3& b) {
        A = a;
        B = b;
        center = (a + b) * 0.5f;
        dir = glm::normalize(b - a);
        up = glm::vec3(0, 1, 0);
        length = glm::distance(A, B);
		//std::cout << "length: " << length << std::endl;
        glm::quat q = rotate();
        Trans.setOrigin(btVector3(center.x, center.y, center.z));
        Trans.setRotation(btQuaternion(q.x, q.y, q.z, q.w));
        Object.setWorldTransform(Trans);

        Body->setWorldTransform(Trans);
        MotionState->setWorldTransform(Trans);
    }
    

   
    glm::quat rotate() {
        float cosTheta = glm::dot(up, dir);
        glm::vec3 rotationAxis;
        glm::quat q;
        if (cosTheta < -0.9999f) {
            rotationAxis = glm::normalize(glm::cross(glm::vec3(1, 0, 0), up));
            if (glm::length(rotationAxis) < 0.01f)
                rotationAxis = glm::normalize(glm::cross(glm::vec3(0, 0, 1), up));
            q = glm::angleAxis(glm::pi<float>(), rotationAxis);
        }
        else if (cosTheta > 0.9999f) {
            q = glm::quat(1, 0, 0, 0);
        }
        else {
            rotationAxis = glm::normalize(glm::cross(up, dir));
            float angle = acosf(cosTheta);
            q = glm::angleAxis(angle, rotationAxis);
        }
		return q;
    }
};



inline bool MeshInside(const btVector3& testPoint, btCollisionWorld* collisionWorld) {
    btVector3 from = testPoint;
    btVector3 to = from + btVector3(0, 10000, 0); 
    btCollisionWorld::AllHitsRayResultCallback rayCallback(from, to);
    collisionWorld->rayTest(from, to, rayCallback);
    return (rayCallback.m_hitPointWorld.size() % 2 == 1);
}


struct ForcepControl {
    bool moveForward = false;
    bool moveBackward = false;
    bool moveLeft = false;
    bool moveRight = false;
    bool moveUp = false;
    bool moveDown = false;
    bool rotateLeft = false;
    bool rotateRight = false;
    bool rotateUp = false;
    bool rotateDown = false;
    bool rollLeft = false;
    bool rollRight = false;
    bool openClaw = false;
    bool closeClaw = false;
};

class Camera {
public:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    float yaw;
    float pitch;
    float movementSpeed;
    float mouseSensitivity;
    float zoom;

    Camera(glm::vec3 pos = glm::vec3(0.0f, 0.0f, 3.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = -90.0f, float pitch = -70.0f) :
        position(pos), worldUp(up), yaw(yaw), pitch(pitch),
        movementSpeed(2.5f), mouseSensitivity(0.1f), zoom(45.0f) {
        updateCameraVectors();
    }

    glm::mat4 getViewMatrix() {
        return glm::lookAt(position, position + front, up);
    }

    void processKeyboard(int direction, float deltaTime) {
        float velocity = movementSpeed * deltaTime;
        if (direction == 0) // FORWARD
            position += front * velocity;
        if (direction == 1) // BACKWARD
            position -= front * velocity;
        if (direction == 2) // LEFT
            position -= right * velocity;
        if (direction == 3) // RIGHT
            position += right * velocity;
        if (direction == 4) // UP
            position += up * velocity;
        if (direction == 5) // DOWN
            position -= up * velocity;
    }

    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) {
        xoffset *= mouseSensitivity;
        yoffset *= mouseSensitivity;

        yaw += xoffset;
        pitch += yoffset;

        if (constrainPitch) {
            if (pitch > 0.0f)
                pitch = 0.0f;
            if (pitch < -89.0f)
                pitch = -89.0f;
        }

        updateCameraVectors();
    }

private:
    void updateCameraVectors() {
        glm::vec3 frontVec;
        frontVec.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        frontVec.y = sin(glm::radians(pitch));
        frontVec.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(frontVec);

        right = glm::normalize(glm::cross(front, worldUp));
        up = glm::normalize(glm::cross(right, front));
    }
};


#pragma once
static GLuint squareVAO = 0, squareVBO = 0, squareEBO = 0;
static GLuint squareShader = 0;
static const char* squareVert = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";
static const char* squareFrag = R"(
#version 330 core
out vec4 FragColor;
uniform vec3 color;
void main() {
    FragColor = vec4(color, 1.0);
}
)";

inline GLuint CompileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    return shader;
}
inline GLuint CreateProgram(const char* vs, const char* fs) {
    GLuint v = CompileShader(GL_VERTEX_SHADER, vs);
    GLuint f = CompileShader(GL_FRAGMENT_SHADER, fs);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, v);
    glAttachShader(prog, f);
    glLinkProgram(prog);
    glDeleteShader(v);
    glDeleteShader(f);
    return prog;
}


inline void InitSquare() {
    if (squareVAO) return;
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f
    };
    unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };
    glGenVertexArrays(1, &squareVAO);
    glGenBuffers(1, &squareVBO);
    glGenBuffers(1, &squareEBO);
    glBindVertexArray(squareVAO);
    glBindBuffer(GL_ARRAY_BUFFER, squareVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, squareEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    squareShader = CreateProgram(squareVert, squareFrag);
};


inline void DrawSquare(
    const glm::vec3& center,
    float length,
    const glm::vec3& color,
    const glm::mat4& view,
    const glm::mat4& projection)
{
    if (!squareVAO) InitSquare();
    glUseProgram(squareShader);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), center) *
        glm::scale(glm::mat4(1.0f), glm::vec3(length, length, 1.0f));
    glUniformMatrix4fv(glGetUniformLocation(squareShader, "model"), 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(squareShader, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(squareShader, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniform3fv(glGetUniformLocation(squareShader, "color"), 1, &color[0]);
    glBindVertexArray(squareVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}


#pragma once
static unsigned int arrowVAO = 0, arrowVBO = 0;
static unsigned int arrowShader = 0;

static const char* arrowVert = R"(#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

static const char* arrowFrag = R"(#version 330 core
out vec4 FragColor;
uniform vec3 color;
void main() {
    FragColor = vec4(color, 1.0);
}
)";

inline void InitArrow() {
    if (arrowVAO) return;

    glGenVertexArrays(1, &arrowVAO);
    glGenBuffers(1, &arrowVBO);
    glBindVertexArray(arrowVAO);
    glBindBuffer(GL_ARRAY_BUFFER, arrowVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6, nullptr, GL_DYNAMIC_DRAW); // 2 points * 3 floats
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    arrowShader = CreateProgram(arrowVert, arrowFrag);
}

inline void DrawArrow(
    const glm::vec3& start,
    const glm::vec3& end,
    const glm::vec3& color,
    const glm::mat4& view,
    const glm::mat4& projection)
{
    if (!arrowVAO) InitArrow();

    float vertices[] = {
        start.x, start.y, start.z,
        end.x,   end.y,   end.z
    };

    glUseProgram(arrowShader);
    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(arrowShader, "model"), 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(arrowShader, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(arrowShader, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniform3fv(glGetUniformLocation(arrowShader, "color"), 1, &color[0]);

    glBindBuffer(GL_ARRAY_BUFFER, arrowVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glBindVertexArray(arrowVAO);
    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);

    glUseProgram(0);
}


