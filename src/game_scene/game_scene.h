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
    std::function<void(GameObject&)> callBack = [](GameObject&) {};
    
    void update() { callBack(*this); }
};

class GameScene
{
public:
    static GameScene& instance();

    void init();
    void update(float dt);

    std::vector<GameObject>& getObjects() { return m_gameObjects; }
    const std::vector<GameObject>& getObjects() const { return m_gameObjects; }

protected:
    GameScene();

private:
    std::vector<GameObject> m_gameObjects;
};