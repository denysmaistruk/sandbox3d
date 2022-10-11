#pragma once
#include "cyclone/contacts.h"

namespace cyclone
{
    class CollisionSphere;
    class CollisionBox;
    class CollisionPlane;
    struct CollisionData;
}

class PhysManager
{
public:
    const static unsigned maxContacts = 256;

    static PhysManager& instance();
    ~PhysManager();

    void update(float dt);

    void addSphere(cyclone::CollisionSphere* sphere);
    void removeSphere(const cyclone::CollisionSphere* sphere);

    void addBox(cyclone::CollisionBox* box);
    void removeBox(const cyclone::CollisionBox* box);

    void addPlane(cyclone::CollisionPlane* plane);
    void removePlane(const cyclone::CollisionPlane* plane);

protected:
    PhysManager();

private:

    void generateContacts();
    void updateObjects(const double dt);

    // Colliders
    std::vector<cyclone::CollisionSphere*> m_spheres;
    std::vector<cyclone::CollisionBox*> m_boxes;
    std::vector<cyclone::CollisionPlane*> m_planes;

    // Contacts
    cyclone::ContactResolver m_contactResolver;
    cyclone::Contact m_contacts[maxContacts];

    // Collision data
    cyclone::CollisionData* m_collisionData;
};