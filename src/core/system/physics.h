#pragma once
#include "system_base.h"
#include "cyclone/contacts.h"

#include "debug/system_debugger.h"

namespace cyclone {
    struct CollisionData;
}

class PhysSystem : public SystemBase<PhysSystem>
{
    friend class SystemDebugger;
    //friend class SystemDebugger2<SystemBase<PhysSystem>>;
public:
    const static unsigned maxContacts = 256;
    int m_substeps;
    float m_updateRate;

    PhysSystem();

    void update(float dt);

    void setCollisionFriction(const float friction);
    float getCollisionFriction() const;
    
    void setCollisionRestitution(const float restitution);
    float getCollisionRestitution() const;
    
    void setCollisionTolerance(const float tolerance);
    float getCollisionTolerance() const;

    // Debug
    float getSleepEpsilon() const { cyclone::getSleepEpsilon(); }
    const cyclone::Contact* getContacts() const { return m_contacts; }
    unsigned getContactCount() const;
    unsigned getRigidBodiesCount() const;
    unsigned getStaticBodiesCount() const;
    unsigned getSleepingCount() const;

private:

    void updateObjects(const double dt);
    void generateContacts();

    // Contacts
    cyclone::ContactResolver m_contactResolver;
    cyclone::Contact m_contacts[maxContacts];

    // Collision data
    cyclone::CollisionData* m_collisionData;
};