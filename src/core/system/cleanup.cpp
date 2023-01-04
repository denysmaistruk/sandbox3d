#include "cleanup.h"

#include "core/component/components.h"
#include "core/system/physics/collision.h"

void CleanupSystem::update(float dt)
{
    auto entityView = getRegistry().view<PhysComponent, RenderComponent, DestroyTag>();

    for (auto [entity, physComponent, renderComponent] : entityView.each())
    {
        // Free physics resources
        CollisionBody* collBody = physComponent.collBody;
        assert(collBody);

        if (auto* collPrimitive = collBody->getPrimitive())
        {
            assert(collPrimitive->body);
            delete collPrimitive->body;
            delete collPrimitive;
        }
        delete collBody;

        // Free render resources
        UnloadModel(renderComponent.model);

        getRegistry().destroy(entity);
    }
}