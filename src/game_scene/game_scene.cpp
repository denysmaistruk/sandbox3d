#include "game_scene.h"

#include "cyclone/collide_fine.h"
#include "phys_manager.h"

#include "raymath.h"

GameScene::GameScene()
{
}

GameScene& GameScene::instance()
{
    static GameScene scene;
    return scene;
}

void GameScene::init()
{
    // Create and set the test box
    cyclone::CollisionBox* box = new cyclone::CollisionBox();
    box->halfSize = cyclone::Vector3(0.5, 0.5, 0.5);
    box->body = new cyclone::RigidBody();
    box->body->setPosition(0, 6, 0);
    box->body->setOrientation(1, 0, 0, 0);
    box->body->setVelocity(0, 0, 0);
    box->body->setRotation(0, 0, 0);
    box->body->setMass(10.0f);
    cyclone::Matrix3 it;
    it.setBlockInertiaTensor(box->halfSize, 100.0f);
    box->body->setInertiaTensor(it);
    box->body->setDamping(0.9f, 0.9f);
    box->body->calculateDerivedData();
    box->calculateInternals();
    box->body->setAcceleration(cyclone::Vector3::GRAVITY);
    box->body->setAwake(true);
    box->body->setCanSleep(true);

    m_gameObjects.emplace_back(GameObject{ 
        LoadModelFromMesh(GenMeshCube(box->halfSize[0] * 2.f, box->halfSize[1] * 2.f, box->halfSize[2] * 2.f))});
    m_gameObjects.back().model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = RED;
    m_gameObjects.back().physBody = box;

    PhysManager::instance().addBox(box);

    // Create and set the test sphere
    cyclone::CollisionSphere* sphere = new cyclone::CollisionSphere();
    sphere->radius = 0.25;
    sphere->body = new cyclone::RigidBody();
    sphere->body->setPosition(0, 8.0, 0.0);
    sphere->body->setOrientation(1, 0, 0, 0);
    sphere->body->setVelocity(0, 1, 0);
    sphere->body->setRotation(0, 0, 0);
    sphere->body->calculateDerivedData();
    sphere->body->setAwake(true);
    sphere->calculateInternals();
    sphere->body->setMass(1.0f);
    sphere->body->setDamping(0.9f, 0.9f);
    it = cyclone::Matrix3();
    it.setDiagonal(5.0f, 5.0f, 5.0f);
    sphere->body->setInertiaTensor(it);
    sphere->body->setAcceleration(cyclone::Vector3::GRAVITY);
    sphere->body->setCanSleep(true);
    sphere->body->setAwake(true);

    m_gameObjects.emplace_back(GameObject{ LoadModelFromMesh(GenMeshSphere(sphere->radius, 32, 32))});
    m_gameObjects.back().model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = RED;
    m_gameObjects.back().physBody = sphere;

    PhysManager::instance().addSphere(sphere);

    // Create the ground plane data
    cyclone::CollisionPlane* plane = new cyclone::CollisionPlane();
    plane->direction = cyclone::Vector3(0, 1, 0);
    plane->offset = 0;

    Image checked = GenImageChecked(100, 100, 1, 1, GRAY, LIGHTGRAY);
    Texture2D checkersTexture = LoadTextureFromImage(checked);
    UnloadImage(checked);
    m_gameObjects.emplace_back(GameObject{ LoadModelFromMesh(GenMeshPlane(100.0f, 100.0f, 1, 1))});
    m_gameObjects.back().model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = checkersTexture;
    m_gameObjects.back().physBody = nullptr;

    PhysManager::instance().addPlane(plane);

    // Create torus, no rigid body
    //m_gameObjects.emplace_back(GameObject{ LoadModelFromMesh(GenMeshTorus(0.3f, 2.f, 20.f, 20.f)) });
    //m_gameObjects.back().model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = RED;
    //m_gameObjects.back().physBody = nullptr;
    //
    //m_gameObjects.back().callBack = [](GameObject& obj) {
    //    static Vector3 rotPhase = Vector3Zero();
    //    rotPhase = Vector3Add(rotPhase, Vector3{ 0.01f, 0.02f, 0.03f });
    //    Matrix rotation = MatrixRotateXYZ(rotPhase);
    //    Matrix translate = MatrixTranslate(5, 5, 5);
    //    obj.model.transform = MatrixMultiply(rotation, translate);
    //};
}

void GameScene::update(float dt)
{
    // Update physics
    PhysManager::instance().update(dt);

    // Update game objects positions
    for (auto& obj : m_gameObjects) 
    {
        // User update
        obj.update();

        if (!obj.physBody) {
            continue;
        }
        
        // Update transform
        const cyclone::Matrix4 physBodyTransform = obj.physBody->body->getTransform();
        Matrix& modelTransform = obj.model.transform;

        modelTransform.m0 = physBodyTransform.data[0];
        modelTransform.m4 = physBodyTransform.data[1];
        modelTransform.m8 = physBodyTransform.data[2];
        modelTransform.m12 = physBodyTransform.data[3];

        modelTransform.m1 = physBodyTransform.data[4];
        modelTransform.m5 = physBodyTransform.data[5];
        modelTransform.m9 = physBodyTransform.data[6];
        modelTransform.m13 = physBodyTransform.data[7];

        modelTransform.m2 = physBodyTransform.data[8];
        modelTransform.m6 = physBodyTransform.data[9];
        modelTransform.m10 = physBodyTransform.data[10];
        modelTransform.m14 = physBodyTransform.data[11];

        modelTransform.m3 = 0;
        modelTransform.m7 = 0;
        modelTransform.m11 = 0;
        modelTransform.m15 = 1;
    }
}