#include "phys_manager.h"

#include "cyclone/collide_fine.h"

PhysManager& PhysManager::instance()
{
    static PhysManager manager;
    return manager;
}

PhysManager::~PhysManager()
{
    for (auto* sphere : m_spheres) {
        delete sphere->body;
        delete sphere;
    }

    for (auto* box : m_boxes) {
        delete box->body;
        delete box;
    }

    for (auto* plane : m_planes) {
        delete plane;
    }
}

void PhysManager::update(float dt)
{
    if (dt <= 0.0f) {
        return;
    }

    if (dt > m_updateRate) {
        dt = m_updateRate;
    }

    // Update the objects
    updateObjects(dt);

    // Perform the contact generation
    generateContacts();

    // Resolve detected contacts
    m_contactResolver.resolveContacts(
        m_collisionData->contactArray,
        m_collisionData->contactCount,
        dt
    );
}

void PhysManager::setUpdateRate(int fps)
{
    m_updateRate = 1.f / fps;
}

void PhysManager::setCollisionFriction(const float friction)
{
    assert(m_collisionData);
    m_collisionData->friction = friction;
}

void PhysManager::setCollisionRestitution(const float restitution)
{
    assert(m_collisionData);
    m_collisionData->restitution = restitution;

}

void PhysManager::setCollisionTolerance(const float tolerance)
{
    assert(m_collisionData);
    m_collisionData->tolerance = tolerance;

}

void PhysManager::addSphere(cyclone::CollisionSphere* sphere)
{
    m_spheres.push_back(sphere);
}

void PhysManager::removeSphere(const cyclone::CollisionSphere* sphere)
{
    if (auto it = std::find(m_spheres.begin(), m_spheres.end(), sphere); it != m_spheres.end()) {
        delete (*it)->body;
        delete *it;
        m_spheres.erase(it);
    }
}

void PhysManager::addBox(cyclone::CollisionBox* box)
{
    m_boxes.push_back(box);
}

void PhysManager::removeBox(const cyclone::CollisionBox* box)
{
    if (auto it = std::find(m_boxes.begin(), m_boxes.end(), box); it != m_boxes.end()) {
        delete (*it)->body;
        delete* it;
        m_boxes.erase(it);
    }
}

void PhysManager::addPlane(cyclone::CollisionPlane* plane)
{
    m_planes.push_back(plane);
}

void PhysManager::removePlane(const cyclone::CollisionPlane* plane)
{
    if (auto it = std::find(m_planes.begin(), m_planes.end(), plane); it != m_planes.end()) {
        delete* it;
        m_planes.erase(it);
    }
}

PhysManager::PhysManager()
    : m_contactResolver(maxContacts * 8)
    , m_collisionData(new cyclone::CollisionData())
    , m_updateRate(0.033f)
{
    m_collisionData->contactArray = m_contacts;
    m_collisionData->friction = 0.5;
    m_collisionData->restitution = 0.3;
    m_collisionData->tolerance = 0.1;
}

void PhysManager::generateContacts()
{
    m_collisionData->reset(maxContacts);

    // boxes
    for (auto* box : m_boxes)
    {
        // box and plane
        for (auto* plane : m_planes)
        {
            if (m_collisionData->hasMoreContacts()) {
                cyclone::CollisionDetector::boxAndHalfSpace(*box, *plane, m_collisionData);
            }
            else {
                return;
            }
        }

        // box and sphere
        for (auto* sphere : m_spheres)
        {
            if (m_collisionData->hasMoreContacts()) {
                cyclone::CollisionDetector::boxAndSphere(*box, *sphere, m_collisionData);
            }
            else {
                return;
            }
        }

        // box and box
        for (auto* otherBox : m_boxes)
        {
            if (m_collisionData->hasMoreContacts()) 
            {
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
    for (auto* sphere : m_spheres)
    {
        // sphere and plane
        for (auto* plane : m_planes)
        {
            if (m_collisionData->hasMoreContacts()) {
                cyclone::CollisionDetector::sphereAndHalfSpace(*sphere, *plane, m_collisionData);
            }
            else {
                return;
            }
        }

        // sphere and sphere
        for (auto* otherSpere : m_spheres)
        {
            if (m_collisionData->hasMoreContacts())
            {
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

void PhysManager::updateObjects(const double dt)
{
    for (auto* sphere : m_spheres)
    {
        sphere->body->integrate(dt);
        sphere->calculateInternals();
    }

    for (auto* box : m_boxes)
    {
        box->body->integrate(dt);
        box->calculateInternals();
    }
}