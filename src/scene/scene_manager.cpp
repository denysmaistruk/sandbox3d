#include "scene_manager.h"

#include "cyclone/collide_fine.h"
#include "phys_manager.h"

#include "../utils/raylib_cyclone_adapter.h"
#include "../utils/imgui_impl_physbox.h"

#include "raymath.h"

SceneManager::SceneManager()
    : m_groundPlaneWidth(90.f)
{
    auto& physManager = PhysManager::instance(); // construct phys manager
    ImGui_ImplPhysbox_Config::substeps = physManager.getSubsteps();
    ImGui_ImplPhysbox_Config::sleepEpsilon = cyclone::getSleepEpsilon();
}

SceneManager& SceneManager::instance()
{
    static SceneManager scene;
    return scene;
}

SceneManager::~SceneManager()
{
}

void SceneManager::init()
{
    spawnBox(Vector3{ 0, 4, 0 }, Vector3Zero(), Vector3{ 0.5, 0.5, 0.5 }, 10.f); // 
    spawnBox(Vector3{ 0, 5, 0 }, Vector3Zero(), Vector3{ 0.5, 0.5, 0.5 }, 10.f); // stack of 3 - ss - 10
    spawnBox(Vector3{ 0, 6, 0 }, Vector3Zero(), Vector3{ 0.5, 0.5, 0.5 }, 10.f); // stack of two boxes 3 substeps is
    spawnBall(Vector3{ 0.f, 8.f, 0.f }, Vector3{ 0.f, 100.f, 1.f }, 0.25f, 1.0f);
    spawnGroundPlane();
    spawnBorderPlanes();
}

void SceneManager::update(float dt)
{
    // Update physics
    PhysManager::instance().update(dt); // speed up simulation

    // Update game objects positions
    for (auto& obj : m_gameObjects) 
    {
        // User update
        obj.update();

        if (obj.physBody) {
            // Update transform by phys body
            obj.model.transform = toRaylib(obj.physBody->body->getTransform());
        }
    }
}

void SceneManager::onThrowBallFromCamera(const Camera3D& camera)
{
    Vector3 velocity = Vector3Subtract(camera.target, camera.position);
    velocity = Vector3Normalize(velocity);
    velocity = Vector3Scale(velocity, 40.f);

    spawnBall(camera.position, velocity, 0.25f, 1.f);
    update(0);
}

void SceneManager::onThrowBoxFromCamera(const Camera3D& camera)
{
    Vector3 velocity = Vector3Subtract(camera.target, camera.position);
    velocity = Vector3Normalize(velocity);
    velocity = Vector3Scale(velocity, 40.f);

    spawnBox(camera.position, velocity, Vector3{ 0.5, 0.5, 0.5 }, 10.f);
    update(0);
}

void SceneManager::spawnBox(const Vector3& pos, const Vector3& velocity, const Vector3& halfSize, const float mass)
{
    // Create and set the test box
    cyclone::CollisionBox* box = new cyclone::CollisionBox();
    box->halfSize = toCyclone(halfSize);
    box->body = new cyclone::RigidBody();
    box->body->setPosition(pos.x, pos.y, pos.z);
    box->body->setOrientation(1, 0, 0, 0);
    box->body->setVelocity(velocity.x, velocity.y, velocity.z);
    box->body->setRotation(0, 0, 0);
    box->body->setMass(mass);
    cyclone::Matrix3 it;
    it.setBlockInertiaTensor(box->halfSize, mass);
    box->body->setInertiaTensor(it);
    box->body->setDamping(0.9f, 0.9f);
    box->body->calculateDerivedData();
    box->calculateInternals();
    box->body->setAcceleration(cyclone::Vector3::GRAVITY);
    box->body->setAwake(true);
    box->body->setCanSleep(true);

    m_gameObjects.emplace_back(GameObject{
        LoadModelFromMesh(GenMeshCube(box->halfSize[0] * 2.f, box->halfSize[1] * 2.f, box->halfSize[2] * 2.f)) });
    m_gameObjects.back().model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = RED;
    m_gameObjects.back().physBody = box;
    m_gameObjects.back().shadowFactor = 0.1f;
    m_gameObjects.back().updateCallBack = [](GameObject& obj)
    {
        if (obj.physBody) {
            auto color = obj.physBody->body->getAwake() ? RED : PURPLE;
            obj.model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = color;
        }
    };

    PhysManager::instance().addBox(box);
}

void SceneManager::spawnBall(const Vector3& pos, const Vector3& velocity, const float radius, const float mass)
{
    cyclone::CollisionSphere* sphere = new cyclone::CollisionSphere();
    sphere->radius = radius;
    sphere->body = new cyclone::RigidBody();
    sphere->body->setPosition(pos.x, pos.y, pos.z);
    sphere->body->setOrientation(1, 0, 0, 0);
    sphere->body->setVelocity(velocity.x, velocity.y, velocity.z);
    sphere->body->setRotation(0, 0, 0);
    sphere->body->calculateDerivedData();
    sphere->body->setAwake(true);
    sphere->calculateInternals();
    sphere->body->setMass(mass);
    sphere->body->setDamping(0.9f, 0.9f);
    cyclone::Matrix3 it = cyclone::Matrix3();
    it.setDiagonal(5.0f, 5.0f, 5.0f);
    sphere->body->setInertiaTensor(it);
    sphere->body->setAcceleration(cyclone::Vector3::GRAVITY);
    sphere->body->setCanSleep(true);
    sphere->body->setAwake(true);

    m_gameObjects.emplace_back(GameObject{ LoadModelFromMesh(GenMeshSphere(sphere->radius, 32, 32)) });
    m_gameObjects.back().model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = RED;
    m_gameObjects.back().physBody = sphere;
    m_gameObjects.back().shadowFactor = 0.1f;
    m_gameObjects.back().updateCallBack = [](GameObject& obj)
    {
        if (obj.physBody) {
            auto color = obj.physBody->body->getAwake() ? RED : PURPLE;
            obj.model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = color;
        }
    };

    PhysManager::instance().addSphere(sphere);
}

void SceneManager::spawnGroundPlane()
{
    cyclone::CollisionPlane* plane = new cyclone::CollisionPlane();
    plane->direction = cyclone::Vector3(0, 1, 0);
    plane->offset = 0;

    Image checked = GenImageChecked(m_groundPlaneWidth, m_groundPlaneWidth, 1, 1, GRAY, LIGHTGRAY);
    Texture2D checkersTexture = LoadTextureFromImage(checked);
    UnloadImage(checked);
    m_gameObjects.emplace_back(GameObject{ LoadModelFromMesh(GenMeshPlane(m_groundPlaneWidth, m_groundPlaneWidth, 1, 1)) });
    m_gameObjects.back().model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = checkersTexture;
    m_gameObjects.back().physBody = nullptr;
    m_gameObjects.back().shadowFactor = 0.5f;

    PhysManager::instance().addPlane(plane);
}

void SceneManager::spawnRotatingTorus(const Vector3& pos, const Vector3& rotVelocity, float radius, float size)
{
    m_gameObjects.emplace_back(GameObject{ LoadModelFromMesh(GenMeshTorus(radius, size, 20.f, 20.f)) });
    m_gameObjects.back().model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = RED;
    m_gameObjects.back().physBody = nullptr;
    m_gameObjects.back().shadowFactor = 0.1f;
    m_gameObjects.back().updateCallBack = [rotVelocity, pos](GameObject& obj) {
        static Vector3 rotPhase = Vector3Zero();
        rotPhase = Vector3Add(rotPhase, rotVelocity);
        Matrix rotation = MatrixRotateXYZ(rotPhase);
        Matrix translate = MatrixTranslate(pos.x, pos.y, pos.z);
        obj.model.transform = MatrixMultiply(rotation, translate);
    };
}

void SceneManager::spawnBorderPlanes()
{
    cyclone::CollisionPlane* plane = new cyclone::CollisionPlane();
    plane->direction = cyclone::Vector3(1, 0, 0);
    plane->offset = m_groundPlaneWidth / -2.f;
    PhysManager::instance().addPlane(plane);

    plane = new cyclone::CollisionPlane();
    plane->direction = cyclone::Vector3(-1, 0, 0);
    plane->offset = m_groundPlaneWidth / -2.f;
    PhysManager::instance().addPlane(plane);

    plane = new cyclone::CollisionPlane();
    plane->direction = cyclone::Vector3(0, 0, 1);
    plane->offset = m_groundPlaneWidth / -2.f;
    PhysManager::instance().addPlane(plane);

    plane = new cyclone::CollisionPlane();
    plane->direction = cyclone::Vector3(0, 0, -1);
    plane->offset = m_groundPlaneWidth / -2.f;
    PhysManager::instance().addPlane(plane);
}

void SceneManager::drawCantacts() const
{
    auto& physManager = PhysManager::instance();

    auto* contacts = physManager.getContacts();
    for (int i = 0; i < physManager.getContactCount(); ++i) {
        // Inter body contacts are in green, floor contacts are red
        auto color = contacts[i].body[1] ? GREEN : RED;

        Vector3 pos = toRaylib(contacts[i].contactPoint);
        Vector3 normal = toRaylib(contacts[i].contactNormal);
        DrawLine3D(pos, Vector3Add(pos, normal), color);
    }
}

void SceneManager::drawSceneBorders() const
{
    float w = getGroundPlaneWidth(), t = 0.1f, h = 25.f;
    auto color = WHITE;

    DrawCubeWires(Vector3{ 0.5f * w + 0.5f * t , 0.5f * h, 0.f }, t, h, w, color); // +x
    DrawCubeWires(Vector3{ -0.5f * w - 0.5f * t , 0.5f * h, 0.f }, t, h, w, color); // -x
    DrawCubeWires(Vector3{ 0.f, 0.5f * h, 0.5f * w + 0.5f * t }, w, h, t, color); // +z
    DrawCubeWires(Vector3{ 0.f, 0.5f * h, -0.5f * w - 0.5f * t }, w, h, t, color); // -z   
}