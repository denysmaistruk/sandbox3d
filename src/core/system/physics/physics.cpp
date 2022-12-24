#include "physics.h"

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
    cyclone::setSleepEpsilon(0.4f);
}

void PhysSystem::update(float dt)
{
    if (dt <= 0.0f) {
        return;
    }

    if (dt > m_params.updateRate) {
        dt = m_params.updateRate;
    }

    for (int i = 0; i < m_params.substeps; ++i) {

        float step = dt / float(m_params.substeps);
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

void PhysSystem::updateObjects(const double dt)
{
    auto enttView = getRegistry().view<PhysComponent, TransformComponent>();
    
    // Reset counters
    m_rigidBodiesCount = 0;
    m_staticBodiesCount = 0;
    m_sleepingBodiesCount = 0;

    for (auto entity : enttView) {
        auto& physComponent = enttView.get<PhysComponent>(entity);
        if (auto* collisionBody = physComponent.collisionBody; collisionBody->body) {
            ++m_rigidBodiesCount;
            if (!collisionBody->body->getAwake()) {
                ++m_sleepingBodiesCount;
            }

            // Update collision body transform
            collisionBody->body->integrate(dt);
            collisionBody->calculateInternals();
            
            // Update transform component
            auto& transformComponent = enttView.get<TransformComponent>(entity);
            transformComponent.transform = toRaylib(collisionBody->body->getTransform());
        }
        else {
            ++m_staticBodiesCount;
        }
    }
}

void PhysSystem::generateContacts()
{
    std::vector<cyclone::CollisionBox*>     boxes;
    std::vector<cyclone::CollisionSphere*>  spheres;
    std::vector<cyclone::CollisionPlane*>   planes;

    auto enttView = getRegistry().view<PhysComponent>();

    for (auto entity : enttView) {
        auto& physComponent = enttView.get<PhysComponent>(entity);
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

void PhysSystem::onParamsChanged()
{
    assert(m_collisionData);
    m_collisionData->friction = m_params.friction;
    m_collisionData->restitution = m_params.restitution;
    m_collisionData->tolerance = m_params.tolerance;
}