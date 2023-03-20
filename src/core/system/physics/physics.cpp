#include "physics.h"
#include "collision.h"

#include "core/component/components.h"
#include "cyclone/collide_fine.h"
#include "utils/raylib_cyclone_adapter.h"
#include "utils/raylib_impl_sandbox3d.h"

PhysSystem::PhysSystem(size_t id)  
    : m_params{ 8, 0.033f, 0.5f, 0.3f, 0.1f }
    , m_contactResolver(maxContacts * 8)
    , m_collisionData(new cyclone::CollisionData{ 
        m_contacts, m_contacts, 0, 0, m_params.friction, m_params.restitution, m_params.tolerance })
    , m_contactsCount(0)
    , m_rigidBodiesCount(0)
    , m_staticBodiesCount(0)
    , m_sleepingBodiesCount(0)
    , m_isUpdatePaused(false)
{
    cyclone::setSleepEpsilon(SANDBOX3D_PHYSICS_SLEEP_EPSILON);
}

void PhysSystem::update(float dt) 
{
    // Update from used input
    updateTrasformClicked(dt);

    if (dt <= 0.0f || m_isUpdatePaused) 
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

    auto entityView = getRegistry().view<PhysComponent, TransformComponent>(entt::exclude<DestroyTag, ClickedEntityTag>);
    
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

            // Add destroy tag for entities out of wold boundaries
            if (!isInWorldBoundaries(toRaylib(collBody->getRigidBody()->getPosition())))
            {
                getRegistry().emplace<DestroyTag>(entity);
            }

            continue;
        }
        // Update statistics
        ++m_staticBodiesCount;   
    }
}

void PhysSystem::updateTrasformClicked(float dt)
{
    auto entityView = getRegistry().view<PhysComponent, TransformComponent, ClickedEntityTag>(entt::exclude<DestroyTag>);
    for (auto [entity, physComponent, transformComponent] : entityView.each())
    {
        if (CollisionBody* body = physComponent.collBody; body->isDynamic())
        {
            Matrix transform = transformComponent.transform;

            cyclone::Vector3 position = toCyclone(vector3Translate(Vector3Zero(), transform));
            cyclone::Quaternion orientation = toCyclone(QuaternionFromMatrix(transform));
            body->getRigidBody()->setPosition(position.x, position.y, position.z);
            body->getRigidBody()->setOrientation(orientation);
            body->getRigidBody()->setAwake(true);
            body->getRigidBody()->calculateDerivedData();
        }
    }
}

void PhysSystem::generateContacts() 
{
    assert(m_collisionData);
    m_collisionData->reset(maxContacts);

    auto entityView = getRegistry().view<PhysComponent>(entt::exclude<DestroyTag, ClickedEntityTag>);

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
    // Update statistics info
    m_contactsCount = m_collisionData->contactCount;
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
    m_contactsCount = 0;
    m_rigidBodiesCount = 0;
    m_staticBodiesCount = 0;
    m_sleepingBodiesCount = 0;
}

bool PhysSystem::isInWorldBoundaries(const Vector3& position)
{
    return position.y > -100.f;
}