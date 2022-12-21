#include "physics.h"

#include "core/component/components.h"
#include "cyclone/collide_fine.h"
#include "utils/raylib_cyclone_adapter.h"

PhysSystem::PhysSystem()
    : m_contactResolver(maxContacts * 8)
    , m_collisionData(new cyclone::CollisionData{ m_contacts, m_contacts, 0, 0, 0.5f, 0.3f, 0.1f })
    , m_updateRate(0.033f)
    , m_substeps(8)
{
    cyclone::setSleepEpsilon(0.4f);
}

void PhysSystem::update(float dt)
{
    if (dt <= 0.0f) {
        return;
    }

    if (dt > m_updateRate) {
        dt = m_updateRate;
    }

    for (int i = 0; i < m_substeps; ++i) {

        float step = dt / float(m_substeps);
        // Update the objects
        updateObjects(step);

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

void PhysSystem::setCollisionFriction(const float friction)
{
    assert(m_collisionData);
    m_collisionData->friction = friction;
}

void PhysSystem::setCollisionRestitution(const float restitution)
{
    assert(m_collisionData);
    m_collisionData->restitution = restitution;
}

void PhysSystem::setCollisionTolerance(const float tolerance)
{
    assert(m_collisionData);
    m_collisionData->tolerance = tolerance;
}

float PhysSystem::getCollisionFriction() const
{
    return m_collisionData->friction;
}

float PhysSystem::getCollisionRestitution() const
{
    return m_collisionData->restitution;
}

float PhysSystem::getCollisionTolerance() const
{
    return m_collisionData->tolerance;
}

unsigned PhysSystem::getContactCount() const
{
    return m_collisionData->contactCount;
}


// TODO: the count can be cached in update
unsigned PhysSystem::getRigidBodiesCount() const
{
    auto view = getRegistry().view<PhysComponent>();

    unsigned count = 0;
    for (auto entity : view) {
        auto& physComponent = view.get<PhysComponent>(entity);
        if (auto* collisionBody = physComponent.collisionBody; collisionBody->body) {
            ++count;
        }
    }
    return count;
}

// TODO: the count can be cached in update
unsigned PhysSystem::getStaticBodiesCount() const
{
    auto view = getRegistry().view<PhysComponent>();

    unsigned count = 0;
    for (auto entity : view) {
        auto& physComponent = view.get<PhysComponent>(entity);
        if (auto* collisionBody = physComponent.collisionBody; collisionBody->body) {
        }
        else {
            ++count;
        }
    }
    return count;
}

// TODO: the count can be cached in update
unsigned PhysSystem::getSleepingCount() const
{
    auto view = getRegistry().view<PhysComponent>();

    unsigned count = 0;
    for (auto entity : view) {
        auto& physComponent = view.get<PhysComponent>(entity);
        if (auto* collisionBody = physComponent.collisionBody; collisionBody->body && !collisionBody->body->getAwake()) {
            ++count;
        }
    }
    return count;
}

void PhysSystem::updateObjects(const double dt)
{
    auto view = getRegistry().view<PhysComponent, TransformComponent>();
    
    for (auto entity : view) {
        auto& physComponent = view.get<PhysComponent>(entity);
        if (auto* collisionBody = physComponent.collisionBody; collisionBody->body) {
            
            // Update collision body transform
            collisionBody->body->integrate(dt);
            collisionBody->calculateInternals();
            
            // Update transform component
            auto& transformComponent = view.get<TransformComponent>(entity);
            transformComponent.transform = toRaylib(collisionBody->body->getTransform());
        }
    }
}

void PhysSystem::generateContacts()
{
    std::vector<cyclone::CollisionBox*>     boxes;
    std::vector<cyclone::CollisionSphere*>  spheres;
    std::vector<cyclone::CollisionPlane*>   planes;

    auto view = getRegistry().view<PhysComponent>();

    for (auto entity : view) {
        auto& physComponent = view.get<PhysComponent>(entity);
        if (auto* collisionBody = physComponent.collisionBody) {
            switch (physComponent.collider) {
            case PhysComponent::ColliderType::Box:
                boxes.push_back(static_cast<cyclone::CollisionBox*>(collisionBody));
                break;
            case PhysComponent::ColliderType::Sphere:
                spheres.push_back(static_cast<cyclone::CollisionSphere*>(collisionBody));
                break;
            case PhysComponent::ColliderType::Plane:
                planes.push_back(physComponent.collisionPlane);
                break;
            }
        }
    }

    // Contacts generation
    m_collisionData->reset(maxContacts);

    // boxes
    for (auto* box : boxes) {
        // box and plane
        for (auto* plane : planes) {
            if (m_collisionData->hasMoreContacts()) {
                cyclone::CollisionDetector::boxAndHalfSpace(*box, *plane, m_collisionData);
            }
            else {
                return;
            }
        }

        // box and sphere
        for (auto* sphere : spheres) {
            if (m_collisionData->hasMoreContacts()) {
                cyclone::CollisionDetector::boxAndSphere(*box, *sphere, m_collisionData);
            }
            else {
                return;
            }
        }

        // box and box
        for (auto* otherBox : boxes) {
            if (m_collisionData->hasMoreContacts()) {
                if (otherBox == box) {
                    continue;
                }
                cyclone::CollisionDetector::boxAndBox(*box, *otherBox, m_collisionData);
            }
            else {
                return;
            }
        }
    }

    // spheres
    for (auto* sphere : spheres) {
        // sphere and plane
        for (auto* plane : planes) {
            if (m_collisionData->hasMoreContacts()) {
                cyclone::CollisionDetector::sphereAndHalfSpace(*sphere, *plane, m_collisionData);
            }
            else {
                return;
            }
        }

        // sphere and sphere
        for (auto* otherSpere : spheres) {
            if (m_collisionData->hasMoreContacts()) {
                if (otherSpere == sphere) {
                    continue;
                }
                cyclone::CollisionDetector::sphereAndSphere(*sphere, *otherSpere, m_collisionData);
            }
            else {
                return;
            }
        }
    }
}