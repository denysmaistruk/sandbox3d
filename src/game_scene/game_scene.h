#pragma once

#include <raylib.h>

#include <functional>

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

class GameScene
{
public:
    static GameScene& instance();
    ~GameScene();

    void init();
    void update(float dt);

    void spawnBox(const Vector3& pos, const Vector3& velocity, const Vector3& halfSize, const float mass);
    void spawnBall(const Vector3& pos, const Vector3& velocity, const float radius, const float mass);
    void spawnGroundPlane();
    void spawnRotatingTorus(const Vector3& pos, const Vector3& rotVelocity, float radius, float size);

    std::vector<GameObject>& getObjects() { return m_gameObjects; }
    const std::vector<GameObject>& getObjects() const { return m_gameObjects; }

protected:
    GameScene();

private:
    std::vector<GameObject> m_gameObjects;
};