#pragma once

namespace action
{
    void bindAllActions();

    // Creating initial entities
    void createScene();

    // New objects
    void throwBallFromCamera();
    void throwBoxFromCamera();
    
    // Camera movement
    void moveCameraToCenter();
    void moveCameraFront();
    void moveCameraBack();
    void moveCameraLeft();
    void moveCameraRight();

    // Selecting objects
    void clickEntity();
}