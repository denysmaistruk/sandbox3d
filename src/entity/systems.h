#pragma once
#include "registry.h"
#include "cyclone/contacts.h"

template<class System>
class SystemBase
{
public:
    static System& get();
    virtual void update(float dt);

protected:
    entt::registry& getRegistry();
    const entt::registry& getRegistry() const;
};

template<class System>
System& SystemBase<System>::get()
{
    static System system;
    return system;
}
template<class System>
void SystemBase<System>::update(float dt)
{
}

template<class System>
entt::registry& SystemBase<System>::getRegistry()
{
    return EntityRegistry::getRegistry();
}

template<class System>
const entt::registry& SystemBase<System>::getRegistry() const
{
    return EntityRegistry::getRegistry();
}

// Render 
//--------------------------------------------------------------------------------------

class RenderSystem : public SystemBase<RenderSystem>
{
public:
    void update(float dt) override;
};

// Physics 
//--------------------------------------------------------------------------------------
namespace cyclone {
    struct CollisionData;
}

class PhysSystem : public SystemBase<PhysSystem>
{
public:
    const static unsigned maxContacts = 256;

    int m_substeps;
    float m_updateRate;

    void update(float dt) override;

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