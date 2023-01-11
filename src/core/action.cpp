#include "action.h"

#include "raylib.h"
#include "raymath.h"

#include "core/camera/camera_controller.h"
#include "core/component/components.h"
#include "core/component/light.h"
#include "core/factory/factory.h"
#include "core/system/render/render.h"
#include "core/system/input/input.h"
#include "utils/imgui_impl_sandbox3d.h"
#include "utils/raylib_impl_sandbox3d.h"

namespace action
{

void bindAllActions()
{
    auto& inputSystem = InputSystem::getSystem();

    inputSystem.addInputEventHandler({ InputEventType::KeyPressed, KEY_E }, [](const InputEvent&) { action::throwBallFromCamera(); });
    inputSystem.addInputEventHandler({ InputEventType::KeyPressed, KEY_R }, [](const InputEvent&) { action::throwBoxFromCamera(); });
    inputSystem.addInputEventHandler({ InputEventType::KeyDown, KEY_Z }, [](const InputEvent&) { action::moveCameraToCenter(); });
    inputSystem.addInputEventHandler({ InputEventType::KeyDown, KEY_W }, [](const InputEvent&) { action::moveCameraFront(); });
    inputSystem.addInputEventHandler({ InputEventType::KeyDown, KEY_S }, [](const InputEvent&) { action::moveCameraBack(); });
    inputSystem.addInputEventHandler({ InputEventType::KeyDown, KEY_A }, [](const InputEvent&) { action::moveCameraLeft(); });
    inputSystem.addInputEventHandler({ InputEventType::KeyDown, KEY_D }, [](const InputEvent&) { action::moveCameraRight(); });
    inputSystem.addInputEventHandler({ InputEventType::MouseButtonPressed, MOUSE_BUTTON_LEFT }, [](const InputEvent&) { action::clickEntity(); });
}

void createScene()
{
    // New collision entities
    EntityFactory::createBox(Vector3{ 0, 4, 0 }, Vector3Zero(), Vector3{ 0.5, 0.5, 0.5 }, 10.f);
    EntityFactory::createBox(Vector3{ 0, 5, 0 }, Vector3Zero(), Vector3{ 0.5, 0.5, 0.5 }, 10.f);
    EntityFactory::createBox(Vector3{ 0, 6, 0 }, Vector3Zero(), Vector3{ 0.5, 0.5, 0.5 }, 10.f);
    EntityFactory::createSphere(Vector3{ 0.f, 8.f, 0.f }, Vector3{ 0.f, 100.f, 1.f }, 0.25f, 1.0f);
    EntityFactory::createPlane(Vector3{ 0.f, 1.f, 0.f }, 0.f);

    // New light entities
    constexpr float cutoff  = 0.7f;
    constexpr float radius  = 10.f;
    constexpr float softness= 0.65f;
    EntityFactory::create(LightSource{}, ShadowCaster{} , GRAY  , Position{ 20.f, 70.f, 0.f }   , LookAt{}, DirectionalLight{});
    EntityFactory::create(LightSource{}, ShadowCaster{} , WHITE , Position{ -2.f, 2.f, -2.f }   , LookAt{}  , SpotLight { radius, softness, cutoff });
    EntityFactory::create(LightSource{}, ShadowCaster{} , RED   , Position{ 2.f, 2.f, 2.f }     , LookAt{}  , SpotLight { radius, softness, cutoff });
    EntityFactory::create(LightSource{}                 , YELLOW, Position{ 4.f, 1.f, 4.f }     , PointLight{ radius, softness });
}

void throwBallFromCamera()
{
    Camera& camera = CameraController::getCamera();
    Vector3 velocity = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    velocity = Vector3Scale(velocity, 40.f);

    EntityFactory::createSphere(camera.position, velocity, 0.25f, 1.f);
}

void throwBoxFromCamera()
{
    Camera& camera = CameraController::getCamera();
    Vector3 velocity = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    velocity = Vector3Scale(velocity, 40.f);

    EntityFactory::createBox(camera.position, velocity, Vector3{ 0.5, 0.5, 0.5 }, 10.f);
}

void moveCameraToCenter()
{
    CameraController::getCamera().target = Vector3Zero();
}

void moveCameraFront()
{
    Camera& camera = CameraController::getCamera();
    Vector3 direction = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    camera.target = Vector3Add(camera.target, Vector3Scale(direction, 0.5f));
}

void moveCameraBack()
{
    Camera& camera = CameraController::getCamera();
    Vector3 direction = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    camera.target = Vector3Subtract(camera.target, Vector3Scale(direction, 0.5f));
}

void moveCameraLeft()
{
    Camera& camera = CameraController::getCamera();
    Vector3 fwd = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 direction = Vector3Normalize(Vector3CrossProduct(fwd, camera.up));
    camera.position = Vector3Subtract(camera.position, Vector3Scale(direction, 0.5f));
    camera.target = Vector3Subtract(camera.target, Vector3Scale(direction, 0.5f));
}

void moveCameraRight()
{
    Camera& camera = CameraController::getCamera();
    Vector3 fwd = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 direction = Vector3Normalize(Vector3CrossProduct(fwd, camera.up));
    camera.position = Vector3Add(camera.position, Vector3Scale(direction, 0.5f));
    camera.target = Vector3Add(camera.target, Vector3Scale(direction, 0.5f));
}

void clickEntity()
{
    if (!IsKeyDown(KEY_LEFT_CONTROL))
    {
        return;
    }

    Ray ray = GetMouseRay(GetMousePosition(),CameraController::getCamera());
    RayCollision collision = { false, 100000.f, Vector3Zero(), Vector3Zero()};
    entt::entity collEntity;

    auto& registry = EntityRegistry::getRegistry();
    auto& entityView = registry.view<RenderComponent>(entt::exclude<DestroyTag>);
    
    for (auto [entity, renderComponent] : entityView.each())
    {
        RayCollision collisionTemp = GetRayCollisionBox(ray, GetModelBoundingBoxTransformed(renderComponent.model));
        if (collisionTemp.hit && collisionTemp.distance < collision.distance)
        {
            collision = collisionTemp;
            collEntity = entity;
        }
    }

    auto clearAllClicked = [&registry]() 
    {
        for (auto [entity] : registry.view<ClickedEntityTag>().each())
        {
            registry.remove<ClickedEntityTag>(entity);
        }
    };

    if (collision.hit && registry.valid(collEntity))
    {
        bool clickedBefore = registry.remove<ClickedEntityTag>(collEntity);
        
        if (!clickedBefore)
        {
            clearAllClicked();
            registry.emplace<ClickedEntityTag>(collEntity);
        }
    }
    else
    {
        clearAllClicked();
    }
}

} // action