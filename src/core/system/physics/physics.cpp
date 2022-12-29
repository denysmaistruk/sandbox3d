#include "physics.h"
#include "collision.h"

#include "core/component/components.h"
#include "cyclone/collide_fine.h"
#include "utils/raylib_cyclone_adapter.h"

PhysSystem::PhysSystem()
    : m_params{ 8, 0.033f, 0.5f, 0.3f, 0.1f }
    , m_contactResolver(maxContacts * 8)
    , m_collisionData(new cyclone::CollisionData{ 
        m_contacts, m_contacts, 0, 0, m_params.friction, m_params.restitution, m_params.tolerance })
    , m_rigidBodiesCount(0)
    , m_staticBodiesCount(0)
    , m_sleepingBodiesCount(0)
{
    cyclone::setSleepEpsilon(SANDBOX3D_PHYSICS_SLEEP_EPSILON);
}

void PhysSystem::update(float dt) 
{
    if (dt <= 0.0f) 
    {
        return;
    }

    if (dt > m_params.updateRate) 
    {
        dt = m_params.updateRate;
    }

    for (int i = 0; i < m_params.substeps; ++i) 
    {

        const float step = dt / static_cast<float>(m_params.substeps);
        
        // Update the objects
        updateTransform(step);

        // Perform the contact generation
        generateContacts();

        // Resolve detected contacts
        m_contactResolver.resolveContacts(
            m_collisionData->contactArray,
            m_collisionData->contactCount,
            step
        );
    }
}

void PhysSystem::updateTransform(float dt) 
{
    resetCounters();
    
    auto entityView = getRegistry().view<PhysComponent, TransformComponent>();
   
    for (auto entity : entityView) 
    {
        auto& physComponent = entityView.get<PhysComponent>(entity);
        if (auto* collBody = physComponent.collBody; collBody->isDynamic()) 
        {
            // Update statistics
            ++m_rigidBodiesCount;
            if (collBody->isSleeping()) 
            {
                ++m_sleepingBodiesCount;
            }

            // Update collision body transform
            collBody->integrate(dt);
            
            // Update transform component
            auto& transformComponent = entityView.get<TransformComponent>(entity);
            transformComponent.transform = toRaylib(collBody->getTransform());
            continue;
        }
        // Update statistics
        ++m_staticBodiesCount;   
    }
}

void PhysSystem::generateContacts() 
{
    assert(m_collisionData);
    m_collisionData->reset(maxContacts);

    auto entityView = getRegistry().view<PhysComponent>();

    for (auto ent1 : entityView) 
    {
        for (auto ent2 : entityView) 
        {
            if (ent1 == ent2) 
            {
                continue;
            }
            auto& body1 = entityView.get<PhysComponent>(ent1).collBody;
            auto& body2 = entityView.get<PhysComponent>(ent2).collBody;
            if (body1 && body2) 
            {
                // Collision resolution
                body1->collide(*body2);
            }
        }
    }
}

void PhysSystem::onParamsChanged() 
{
    assert(m_collisionData);
    m_collisionData->friction = m_params.friction;
    m_collisionData->restitution = m_params.restitution;
    m_collisionData->tolerance = m_params.tolerance;
}

void PhysSystem::resetCounters() 
{
    m_rigidBodiesCount = 0;
    m_staticBodiesCount = 0;
    m_sleepingBodiesCount = 0;
}