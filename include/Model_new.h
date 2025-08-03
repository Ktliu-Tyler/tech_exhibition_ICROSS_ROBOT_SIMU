#pragma once
#ifndef MODEL_H
#define MODEL_H
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <GL/glew.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <opencv2/opencv.hpp>

#include <Mesh_m.h>
#include <shader.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <memory>
using namespace std;

// 變換組件結構，分離位置、旋轉、縮放
struct Transform {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);  // 歐拉角 (度)
    glm::vec3 scale = glm::vec3(1.0f);
    

    // 取得變換矩陣
    glm::mat4 getMatrix() const {
        glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 R = getRotationMatrix(rotation);
        glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
        return T * R * S;
    }

    // 取得旋轉矩陣
    glm::mat4 getRotationMatrix(const glm::vec3& angle) const {
        glm::mat4 rot(1.0f);
        rot = glm::rotate(rot, glm::radians(angle.z), glm::vec3(0, 0, 1));
        rot = glm::rotate(rot, glm::radians(angle.y), glm::vec3(0, 1, 0));
        rot = glm::rotate(rot, glm::radians(angle.x), glm::vec3(1, 0, 0));
        return rot;
    }
};

// 紋理載入函數
inline unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false) {
    string filename = string(path);
    filename = directory + '/' + filename;

    std::cout << "Loading texture: " << filename << std::endl;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    cv::Mat image = cv::imread(filename.c_str(), cv::IMREAD_UNCHANGED);
    if (image.empty()) {
        std::cout << "Texture failed to load at path: " << filename << std::endl;
        return 0;
    }

    // OpenCV 影像處理
    cv::flip(image, image, 1);
    cv::flip(image, image, 0);
    cv::cvtColor(image, image, cv::COLOR_BGR2RGB);

    int width = image.cols;
    int height = image.rows;
    int nrComponents = image.channels();

    if (image.data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image.data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
    }

    return textureID;
}

class Model {
public:
    // 基本模型資料
    vector<Texture> textures_loaded;
    vector<Mesh> meshes;
    string directory;
    bool gammaCorrection;
    bool scaleRotateUpdate = false;
    bool posUpdate = false;
    glm::vec3 prePos = glm::vec3(0.0f);
    glm::vec3 oPos = glm::vec3(0.0f);

    // 層次結構
    Model* parent = nullptr;
    vector<shared_ptr<Model>> children;

    // 變換資料
    Transform localTransform;     // 相對於父物件的變換
    mutable glm::mat4 worldMatrix = glm::mat4(1.0f);  // 世界變換矩陣 (緩存)
    mutable bool worldMatrixDirty = true;             // 世界矩陣是否需要更新

    // 處理Mesh
    btTriangleMesh* bulletTriMesh = nullptr;
    btBvhTriangleMeshShape* bulletMeshShape = nullptr;
    btCollisionObject* bulletCollisionObject = nullptr;
    btTransform trans;


    Model() : gammaCorrection(false) {}

    // 建構函數
    Model(const string& path, float scale = 1.0f,
        const glm::vec3& pos = glm::vec3(0.0f),
        const glm::vec3& angle = glm::vec3(0.0f),
        bool gamma = false) : gammaCorrection(gamma) {

        localTransform.position = pos;
        prePos = pos;
		oPos = pos;
        localTransform.rotation = angle;
        localTransform.scale = glm::vec3(scale);
        loadModel(path);
        markWorldMatrixDirty();
        
    }

    // === 變換操作 ===

    // 設置本地位置
    void setLocalPosition(const glm::vec3& pos) {
        localTransform.position = pos;
        markWorldMatrixDirty();
    }

	void setRelatedLocalPosition(const glm::vec3& pos) {
        localTransform.position += pos;
        markWorldMatrixDirty();
	}

    // 設置本地旋轉 (度)
    void setLocalRotation(const glm::vec3& rot) {
        localTransform.rotation = rot;
        markWorldMatrixDirty();
    }

    void setRelatedLocalRotation(const glm::vec3& rot) {
        localTransform.rotation += rot;
        markWorldMatrixDirty();
    }

    // 設置本地縮放
    void setLocalScale(const glm::vec3& scale) {
        localTransform.scale = scale;
        markWorldMatrixDirty();
    }

    void setLocalScale(float uniformScale) {
        setLocalScale(glm::vec3(uniformScale));
    }

    // 獲取本地變換
    const glm::vec3& getLocalPosition() const { return localTransform.position; }
    const glm::vec3& getLocalRotation() const { return localTransform.rotation; }
    const glm::vec3& getLocalScale() const { return localTransform.scale; }

    // 本地變換增量操作
    void translate(const glm::vec3& delta) {
        localTransform.position += delta;
        markWorldMatrixDirty();
		posUpdate = true;
    }

    void rotate(const glm::vec3& deltaAngle) {
        localTransform.rotation += deltaAngle;
        markWorldMatrixDirty();
		scaleRotateUpdate = true;
    }

    void scale(const glm::vec3& scaleFactor) {
        localTransform.scale *= scaleFactor;
        markWorldMatrixDirty();
		scaleRotateUpdate = true;
    }

    // === 世界坐標系操作 ===

    // 獲取世界變換矩陣
    const glm::mat4& getWorldMatrix() const {
        if (worldMatrixDirty) {
            updateWorldMatrix();
        }
        return worldMatrix;
    }

    // 獲取世界位置
    glm::vec3 getWorldPosition() const {
        const glm::mat4& world = getWorldMatrix();
        return glm::vec3(world[3]);
    }

    // 獲取世界旋轉 (提取歐拉角)
    glm::vec3 getWorldRotation() const {
        const glm::mat4& world = getWorldMatrix();
        glm::vec3 scale, translation, skew;
        glm::vec4 perspective;
        glm::quat orientation;

        glm::decompose(world, scale, orientation, translation, skew, perspective);

        // 將四元數轉換為歐拉角
        glm::vec3 eulerAngles = glm::degrees(glm::eulerAngles(orientation));
        return eulerAngles;
    }

    // 獲取世界縮放
    glm::vec3 getWorldScale() const {
        const glm::mat4& world = getWorldMatrix();
        glm::vec3 scale, translation, skew;
        glm::vec4 perspective;
        glm::quat orientation;

        glm::decompose(world, scale, orientation, translation, skew, perspective);
        return scale;
    }

    // 設置世界位置 (會影響本地變換)
    void setWorldPosition(const glm::vec3& worldPos) {
        if (parent) {
            glm::mat4 parentWorldInverse = glm::inverse(parent->getWorldMatrix());
            glm::vec4 localPos = parentWorldInverse * glm::vec4(worldPos, 1.0f);
            setLocalPosition(glm::vec3(localPos));
        }
        else {
            setLocalPosition(worldPos);
        }
		posUpdate = true;
    }

    void setRelatedWorldPosition(const glm::vec3& worldPos) {
        if (parent) {
            glm::mat4 parentWorldInverse = glm::inverse(parent->getWorldMatrix());
            glm::vec4 localPos = parentWorldInverse * glm::vec4(worldPos, 1.0f);
            setRelatedLocalPosition(glm::vec3(localPos));
        }
        else {
            setRelatedLocalPosition(worldPos);
        }
		posUpdate = true;
    }

    // === 坐標軸操作 (用於物理和移動) ===

    // 獲取本地坐標軸
    glm::vec3 getLocalAxisX() const {
        glm::mat4 rot = localTransform.getRotationMatrix(localTransform.rotation);
        return glm::normalize(glm::vec3(rot[0]));
    }

    glm::vec3 getLocalAxisY() const {
        glm::mat4 rot = localTransform.getRotationMatrix(localTransform.rotation);
        return glm::normalize(glm::vec3(rot[1]));
    }

    glm::vec3 getLocalAxisZ() const {
        glm::mat4 rot = localTransform.getRotationMatrix(localTransform.rotation);
        return glm::normalize(glm::vec3(rot[2]));
    }

    // 獲取世界坐標軸

    glm::vec3 getWorldAxisX() const {
        const glm::mat4& world = getWorldMatrix();
        return glm::normalize(glm::vec3(world[0]));
    }

    glm::vec3 getWorldAxisY() const {
        const glm::mat4& world = getWorldMatrix();
        return glm::normalize(glm::vec3(world[1]));
    }

    glm::vec3 getWorldAxisZ() const {
        const glm::mat4& world = getWorldMatrix();
        return glm::normalize(glm::vec3(world[2]));
    }

    // 沿本地坐標軸移動
    void moveForward(float distance) {
        translate(getLocalAxisZ() * -distance);  // OpenGL中 -Z 是前方
    }

    void moveRight(float distance) {
        translate(getLocalAxisX() * distance);
    }

    void moveUp(float distance) {
        translate(getLocalAxisY() * distance);
    }

    // === 層次結構操作 ===

    // 添加子物件
    void addChild(shared_ptr<Model> child) {
        if (child && child->parent != this) {
            // 從舊父物件移除
            if (child->parent) {
                child->parent->removeChild(child);
            }

            child->parent = this;
            children.push_back(child);
            child->markWorldMatrixDirty();
        }
    }

    // 移除子物件
    void removeChild(shared_ptr<Model> child) {
        auto it = find(children.begin(), children.end(), child);
        if (it != children.end()) {
            (*it)->parent = nullptr;
            children.erase(it);
            child->markWorldMatrixDirty();
        }
    }

    // 設置父物件
    void setParent(Model* newParent) {
        if (parent != newParent) {
            // 計算新的本地變換以保持世界位置
            glm::vec3 currentWorldPos = getWorldPosition();

            parent = newParent;
            markWorldMatrixDirty();

            // 保持世界位置不變
            setWorldPosition(currentWorldPos);
        }
    }

    // 獲取所有子物件
    const vector<shared_ptr<Model>>& getChildren() const {
        return children;
    }

    // === 渲染 ===

    void Draw(Shader& shader) {
        shader.setMat4("model", getWorldMatrix());
        for (unsigned int i = 0; i < meshes.size(); i++) {
            meshes[i].Draw(shader);
        }

        // 渲染所有子物件
        for (auto& child : children) {
            child->Draw(shader);
        }
    }

    // === Bullet Physics 輔助函數 ===

    // 獲取用於 Bullet 的變換 (btTransform)
    void getBulletTransform(float* origin, float* rotation) const {
        glm::vec3 worldPos = getWorldPosition();
        glm::vec3 worldRot = getWorldRotation();

        // 位置
        origin[0] = worldPos.x;
        origin[1] = worldPos.y;
        origin[2] = worldPos.z;

        // 旋轉 (轉換為四元數會更好，這裡簡化為歐拉角)
        rotation[0] = glm::radians(worldRot.x);
        rotation[1] = glm::radians(worldRot.y);
        rotation[2] = glm::radians(worldRot.z);
    }

    // 從 Bullet 更新變換
    void updateFromBullet(const float* origin, const float* rotation) {
        glm::vec3 newWorldPos(origin[0], origin[1], origin[2]);
        glm::vec3 newWorldRot(glm::degrees(rotation[0]),
            glm::degrees(rotation[1]),
            glm::degrees(rotation[2]));

        setWorldPosition(newWorldPos);
        // 注意：這裡可能需要更複雜的旋轉處理
    }

    // 取得所有 mesh 的頂點（世界座標可依需求轉換）
    std::vector<glm::vec3> getAllVertices() const {
        std::vector<glm::vec3> all;
        for (const auto& mesh : meshes) {
            for (const auto& v : mesh.getVertices()) {
                all.push_back(v.Position);
            }
        }
        return all;
    }
    std::vector<unsigned int> getAllIndices() const {
        std::vector<unsigned int> all;
        unsigned int offset = 0;
        for (const auto& mesh : meshes) {
            const auto& idx = mesh.getIndices();
            for (auto i : idx) all.push_back(i + offset);
            offset += mesh.getVertices().size();
        }
        return all;
    }

    btCollisionObject* getBulletCollisionObject() {
        buildBulletMeshShape();
        return bulletCollisionObject;
    }

    void setMeshPosition(const glm::vec3& pos) {
        trans.setIdentity();
        trans.setOrigin(btVector3(pos.x, pos.y, pos.z));
        if (bulletCollisionObject)
            bulletCollisionObject->setWorldTransform(trans);
    }

    void setMeshScale(float scale) {
        // 釋放舊的 shape
        delete bulletCollisionObject;
        delete bulletMeshShape;
        delete bulletTriMesh;
        bulletCollisionObject = nullptr;
        bulletMeshShape = nullptr;
        bulletTriMesh = nullptr;
        // 重新建立
        buildBulletMeshShape();
    }

    void updateMesh(btCollisionWorld *collisionWorld) {
        if (!bulletCollisionObject || scaleRotateUpdate) {
            bulletCollisionObject = getBulletCollisionObject();
            collisionWorld->addCollisionObject(bulletCollisionObject);
			scaleRotateUpdate = false;
        }
		if (posUpdate) {
			// 更新位置
			setMeshPosition(getWorldPosition());
			posUpdate = false;
            collisionWorld->updateSingleAabb(bulletCollisionObject);
		}
    }

private:
    // 標記世界矩陣需要更新
    void markWorldMatrixDirty() const {
        worldMatrixDirty = true;
        // 遞歸標記所有子物件
        for (const auto& child : children) {
            child->markWorldMatrixDirty();
        }
    }

    // 更新世界矩陣
    void updateWorldMatrix() const {
        glm::mat4 localMatrix = localTransform.getMatrix();

        if (parent) {
            worldMatrix = parent->getWorldMatrix() * localMatrix;
        }
        else {
            worldMatrix = localMatrix;
        }

        worldMatrixDirty = false;
    }

    // === 模型載入 (保持原有邏輯) ===

    void loadModel(string const& path) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path + ".obj",
            aiProcess_Triangulate | aiProcess_GenSmoothNormals |
            aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }

        directory = path.substr(0, path.find_last_of('\\'));
        processNode(scene->mRootNode, scene);
    }

    void processNode(aiNode* node, const aiScene* scene) {
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        // 處理頂點
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;
            glm::vec3 vector;

            // 位置
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;

            // 法線
            if (mesh->HasNormals()) {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }

            // 紋理坐標
            if (mesh->mTextureCoords[0]) {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;

                // 切線
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;

                // 副切線
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.Bitangent = vector;
            }
            else {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }

            vertices.push_back(vertex);
        }

        // 處理索引
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        // 處理材質
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

        vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        return Mesh(vertices, indices, textures);
    }

    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName) {
        vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
            aiString str;
            mat->GetTexture(type, i, &str);

            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++) {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
                    textures.push_back(textures_loaded[j]);
                    skip = true;
                    break;
                }
            }

            if (!skip) {
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);
            }
        }
        return textures;
    }

    void buildBulletMeshShape() {
        if (bulletTriMesh) return; // 已建立過
        auto vertices = getAllVertices();
        auto indices = getAllIndices();
        glm::vec3 scale = localTransform.scale;
        glm::mat4 rotMat = localTransform.getRotationMatrix(localTransform.rotation);

        bulletTriMesh = new btTriangleMesh();
        for (size_t i = 0; i + 2 < indices.size(); i += 3) {
            glm::vec3 v0 = vertices[indices[i]] * scale;
            glm::vec3 v1 = vertices[indices[i + 1]] * scale;
            glm::vec3 v2 = vertices[indices[i + 2]] * scale;
            // 旋轉
            v0 = glm::vec3(rotMat * glm::vec4(v0, 1.0f));
            v1 = glm::vec3(rotMat * glm::vec4(v1, 1.0f));
            v2 = glm::vec3(rotMat * glm::vec4(v2, 1.0f));
            bulletTriMesh->addTriangle(
                btVector3(v0.x, v0.y, v0.z),
                btVector3(v1.x, v1.y, v1.z),
                btVector3(v2.x, v2.y, v2.z)
            );
        }
        bulletMeshShape = new btBvhTriangleMeshShape(bulletTriMesh, true);
        bulletCollisionObject = new btCollisionObject();
        bulletCollisionObject->setCollisionShape(bulletMeshShape);
    }

};

#endif