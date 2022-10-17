#pragma once

#include <raylib.h>
#include "voxel.h"

namespace cyclone
{
    class CollisionPrimitive;
}

struct GameObject
{
    Model model;
    cyclone::CollisionPrimitive* physBody;
    float shadowFactor;
    std::function<void(GameObject&, float dt)> updateCallBack = [](GameObject&, float dt) {};
    void update(float dt) { updateCallBack(*this, dt); }
};

class SceneManager
{
public:
    static SceneManager& instance();
    ~SceneManager();

    void init();
    void update(float dt);

    void onThrowBallFromCamera(const Camera3D& camera);
    void onThrowBoxFromCamera(const Camera3D& camera);

    void spawnBox(const Vector3& pos, const Vector3& velocity, const Vector3& halfSize, const float mass);
    void spawnBall(const Vector3& pos, const Vector3& velocity, const float radius, const float mass);
    void spawnRotatingTorus(const Vector3& pos, const Vector3& rotVelocity, float radius, float size);
    void spawnGroundPlane();
    void spawnBorderPlanes();

    void spawnVoxelTorus(float voxelSize, float R, float r);

    float getGroundPlaneWidth() const { return m_groundPlaneWidth; }

    std::vector<GameObject>& getObjects() { return m_gameObjects; }
    const std::vector<GameObject>& getObjects() const { return m_gameObjects; }

    std::vector<VoxelObject>& getVoxelObjects() { return m_voxelObject; }
    const std::vector<VoxelObject>& getVoxelObjects() const { return m_voxelObject; }
    
    // Debug
    void drawCantacts() const;
    void drawSceneBorders() const;

    // Voxels
    Mesh m_mesh;
    Material m_material;
    Matrix m_transforms[2 * 4096000];
    int m_instances;
    void loadVoxels();



protected:
    SceneManager();

private:
    std::vector<GameObject> m_gameObjects;
    std::vector<VoxelObject> m_voxelObject;
    float m_groundPlaneWidth;
};