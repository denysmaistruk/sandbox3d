#pragma once
#include "core/system/system_base.h"
#include "cyclone/contacts.h"

namespace cyclone {
    struct CollisionData;
}

template<typename System>
struct SystemDebugger;

class PhysSystem : public SystemBase<PhysSystem>
{
    friend struct SystemDebugger<PhysSystem>;
public:
    const static unsigned maxContacts = 256;

    struct Params 
    {
        int substeps;
        float updateRate;
        float friction;
        float restitution;
        float tolerance;
    };
    
    PhysSystem();
    
    void update(float dt);

    const Params& getParams() const { return m_params; }
    void setParams(const Params& params) { m_params = params; onParamsChanged(); }

private:
    void updateObjects(const double dt);
    void generateContacts();
    void onParamsChanged();

    // Simulation parameters
    Params m_params;

    // Contacts
    cyclone::ContactResolver m_contactResolver;
    cyclone::Contact m_contacts[maxContacts];

    // Collision data
    cyclone::CollisionData* m_collisionData;

    // Statistics
    int m_rigidBodiesCount;
    int m_staticBodiesCount;
    int m_sleepingBodiesCount;
};