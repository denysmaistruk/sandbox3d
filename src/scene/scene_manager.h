#pragma once

#include <raylib.h>

namespace cyclone
{
    class CollisionPrimitive;
}

struct GameObject
{
    Model model;
    cyclone::CollisionPrimitive* physBody;
    std::function<void(GameObject&)> updateCallBack = [](GameObject&) {};
    void update() { updateCallBack(*this); }
};

class SceneManager
{
public:
    static SceneManager& instance();
    ~SceneManager();

    void init();
    void update(float dt);

    void OnThrowBallFromCamera(const Camera3D& camera);
    void OnThrowBoxFromCamera(const Camera3D& camera);

    void spawnBox(const Vector3& pos, const Vector3& velocity, const Vector3& halfSize, const float mass);
    void spawnBall(const Vector3& pos, const Vector3& velocity, const float radius, const float mass);
    void spawnRotatingTorus(const Vector3& pos, const Vector3& rotVelocity, float radius, float size);
    void spawnGroundPlane();
    void spawnBorderPlanes();

    float getGroundPlaneWidth() const { return m_groundPlaneWidth; }

    std::vector<GameObject>& getObjects() { return m_gameObjects; }
    const std::vector<GameObject>& getObjects() const { return m_gameObjects; }
    
    // Debug
    void drawCantacts() const;
    void drawSceneBorders() const;
    void syncImGuiInput();

protected:
    SceneManager();

private:
    std::vector<GameObject> m_gameObjects;
    float m_groundPlaneWidth;
};