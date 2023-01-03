#pragma once
#include "core/system/system_base.h"
#include "cyclone/contacts.h"

struct Vector3;
namespace cyclone 
{
    struct CollisionData;
}

template<typename System>
struct SystemDebugger;

class PhysSystem : public SystemBase<PhysSystem>
{
    SANDBOX3D_SYSTEM_CLASS(PhysSystem);
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
    
    void update(float dt);
    bool getIsPaused() const { return m_isUpdatePaused; }
   
    const Params& getParams() const { return m_params; }
    void setParams(const Params& params) { m_params = params; onParamsChanged(); }

    cyclone::CollisionData* getCollisionData() { return m_collisionData; }

protected:
    PhysSystem(size_t id);

private:
    void updateTransform(float dt);
    void generateContacts();
    void onParamsChanged();
    void resetCounters();

    bool isInWorldBoundaries(const Vector3& position);

    // Simulation parameters
    Params m_params;

    // Contacts
    cyclone::ContactResolver m_contactResolver;
    cyclone::Contact m_contacts[maxContacts];

    // Collision data
    cyclone::CollisionData* m_collisionData;

    // Statistics
    int m_contactsCount;
    int m_rigidBodiesCount;
    int m_staticBodiesCount;
    int m_sleepingBodiesCount;

    bool m_isUpdatePaused;
};