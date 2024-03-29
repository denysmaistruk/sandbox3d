#include "factory.h"

#include "core/component/components.h"
#include "core/registry/registry.h"
#include "core/system/physics/collision.h"
#include "core/system/physics/physics.h"
#include "cyclone/collide_fine.h"
#include "utils/raylib_cyclone_adapter.h"
#include "utils/raylib_impl_sandbox3d.h"
#include "raymath.h"

entt::entity EntityFactory::createBox(const Vector3& pos, const Vector3& velocity, const Vector3& halfSize, const float mass)
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

    CollisionBox* collisionBox = new CollisionBox();
    collisionBox->box = box;
    collisionBox->collData = PhysSystem::getSystem().getCollisionData();
    
    // Phys component
    PhysComponent physComponent{ collisionBox };

    // Transform component
    TransformComponent transformComponent{ toRaylib(collisionBox->getTransform()) };

    Model model = LoadModelFromMesh(GenMeshCube(box->halfSize[0] * 2.f, box->halfSize[1] * 2.f, box->halfSize[2] * 2.f));
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = RED;
    model.transform = transformComponent.transform;
    float shadowFactor = 0.125f;

    // Render component
    RenderComponent renderComponent{ model, shadowFactor };

    // Create entity and emplace components
    entt::registry& registry = EntityRegistry::getRegistry();
    entt::entity entity = registry.create();

    registry.emplace<PhysComponent>(entity, physComponent);
    registry.emplace<TransformComponent>(entity, transformComponent);
    registry.emplace<RenderComponent>(entity, renderComponent);

    return entity;
}

entt::entity EntityFactory::createSphere(const Vector3& pos, const Vector3& velocity, const float radius, const float mass)
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

    CollisionSphere* collisionSphere = new CollisionSphere();
    collisionSphere->sphere = sphere;
    collisionSphere->collData = PhysSystem::getSystem().getCollisionData();

    // Phys component
    PhysComponent physComponent{ collisionSphere };

    // Transform component
    TransformComponent transformComponent{ toRaylib(collisionSphere->getTransform()) };

    Model model = LoadModelFromMesh(GenMeshSphere(sphere->radius, 32, 32));
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = RED;
    model.transform = transformComponent.transform;
    float shadowFactor = 0.125f;

    // Render component
    RenderComponent renderComponent{ model, shadowFactor };

    // Create entity and emplace components
    entt::registry& registry = EntityRegistry::getRegistry();
    entt::entity entity = registry.create();

    registry.emplace<PhysComponent>(entity, physComponent);
    registry.emplace<TransformComponent>(entity, transformComponent);
    registry.emplace<RenderComponent>(entity, renderComponent);

    return entity;
}

entt::entity EntityFactory::createPlane(const Vector3& direction, const float offset, const float debugExtend /*= 90.f*/)
{
    cyclone::CollisionPlane* plane = new cyclone::CollisionPlane();
    plane->direction = toCyclone(direction);
    plane->offset = offset;

    CollisionPlane* collisionPlane = new CollisionPlane();
    collisionPlane->plane = plane;
    collisionPlane->collData = PhysSystem::getSystem().getCollisionData();

    // Phys component
    PhysComponent physComponent{ collisionPlane };

    // Transform component
    TransformComponent transformComponent{ MatrixIdentity() };

    Image checked = GenImageChecked(debugExtend, debugExtend, 1, 1, GRAY, LIGHTGRAY);
    Texture2D checkersTexture = LoadTextureFromImage(checked);
    UnloadImage(checked);

    Model model = LoadModelFromMesh(GenMeshPlane(debugExtend, debugExtend, 1, 1));
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = checkersTexture;
    model.transform = transformComponent.transform;
    float shadowFactor = 0.75f;

    // Render component
    RenderComponent renderComponent{ model, shadowFactor };

    // Create entity and emplace components
    entt::registry& registry = EntityRegistry::getRegistry();
    entt::entity entity = registry.create();

    registry.emplace<PhysComponent>(entity, physComponent);
    registry.emplace<TransformComponent>(entity, transformComponent);
    registry.emplace<RenderComponent>(entity, renderComponent);

    return entity;
}

entt::entity EntityFactory::createCapsule(const Vector3& pos, const Vector3& velocity, const float radius, const float height, const float mass)
{
    // Create and set the test capsule
    cyclone::CollisionCapsule* capsule = new cyclone::CollisionCapsule();
    capsule->radius = radius;
    capsule->height = height;
    capsule->body = new cyclone::RigidBody();
    capsule->body->setPosition(pos.x, pos.y, pos.z);
    capsule->body->setOrientation(1, 0, 0, 0);
    capsule->body->setVelocity(velocity.x, velocity.y, velocity.z);
    capsule->body->setRotation(0.1f, 0, 0);
    capsule->body->setMass(mass);
    cyclone::Matrix3 it;
    it.setDiagonal(5.f, 5.f * height / radius, 5.f);
    capsule->body->setInertiaTensor(it);
    capsule->body->setDamping(0.9f, 0.9f);
    capsule->body->calculateDerivedData();
    capsule->calculateInternals();
    capsule->body->setAcceleration(cyclone::Vector3::GRAVITY);
    capsule->body->setAwake(true);
    capsule->body->setCanSleep(true);

    CollisionCapsule* collisionCapsule = new CollisionCapsule();
    collisionCapsule->capsule = capsule;
    collisionCapsule->collData = PhysSystem::getSystem().getCollisionData();

    // Phys component
    PhysComponent physComponent{ collisionCapsule };

    // Transform component
    TransformComponent transformComponent{ MatrixTranslate(pos.x, pos.y, pos.z) };

    //Model model = LoadModelFromMesh(genMeshCapsule(radius, height, 32, 32));
    Model model = genCapsuleModel(radius, height, 32, 32);
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = RED;
    model.transform = transformComponent.transform;
    float shadowFactor = 0.125f;

    // Render component
    RenderComponent renderComponent{ model, shadowFactor };

    // Create entity and emplace components
    entt::registry& registry = EntityRegistry::getRegistry();
    entt::entity entity = registry.create();

    registry.emplace<PhysComponent>(entity, physComponent);
    registry.emplace<TransformComponent>(entity, transformComponent);
    registry.emplace<RenderComponent>(entity, renderComponent);

    return entity;
}
