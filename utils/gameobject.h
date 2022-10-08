#pragma once

#include <raylib.h>

#include <functional>

struct GameObject
{
    Model model;
    Vector3 position;
    std::function<void(GameObject&)> updateFunc = [](GameObject&) {};
    
    void update() { updateFunc(*this); }
};