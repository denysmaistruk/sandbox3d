#include "collision.h"

#include "raylib.h"
#include "raymath.h"
#include "utils/raylib_cyclone_adapter.h"

static void boxBoxCollisionImpl(CollisionBox& box1, CollisionBox& box2, cyclone::CollisionData* collData) 
{
    if (&box1 != &box2 && collData->hasMoreContacts())
    {
        cyclone::CollisionDetector::boxAndBox(*box1.box, *box2.box, collData);
    }
}

static void boxSphereCollisionImpl(CollisionBox& box, CollisionSphere& sphere, cyclone::CollisionData* collData) 
{
    if (collData->hasMoreContacts())
    {
        cyclone::CollisionDetector::boxAndSphere(*box.box, *sphere.sphere, collData);
    }
}

static void boxPlaneCollisionImpl(CollisionBox& box, CollisionPlane& plane, cyclone::CollisionData* collData) 
{
    if (collData->hasMoreContacts())
    {
        cyclone::CollisionDetector::boxAndHalfSpace(*box.box, *plane.plane, collData);
    }
}

static void sphereSphereCollisionImpl(CollisionSphere& sphere1, CollisionSphere& sphere2, cyclone::CollisionData* collData) 
{
    if (&sphere1 != &sphere2 && collData->hasMoreContacts())
    {
        cyclone::CollisionDetector::sphereAndSphere(*sphere1.sphere, *sphere2.sphere, collData);
    }
}

static void spherePlaneCollisionImpl(CollisionSphere& sphere, CollisionPlane& plane, cyclone::CollisionData* collData) 
{
    if (collData->hasMoreContacts())
    {
        cyclone::CollisionDetector::sphereAndHalfSpace(*sphere.sphere, *plane.plane, collData);
    }
}

static void capsulePlaneCollisionImpl(CollisionCapsule& capsule, CollisionPlane& plane, cyclone::CollisionData* collData)
{
    const cyclone::Quaternion capsuleOrient = capsule.capsule->body->getOrientation();
    const cyclone::Vector3 capsulePos = capsule.capsule->body->getPosition();

    // Upper ball
    if (collData->hasMoreContacts())
    {
        const auto ballPos = toCyclone(Vector3RotateByQuaternion(Vector3{ 0, 0.5f * capsule.capsule->height, 0 }
            , toRaylib(capsuleOrient))) + capsulePos;

        if (const float ballDist = plane.plane->direction * ballPos - capsule.capsule->radius - plane.plane->offset;
                ballDist < 0)
        {
            cyclone::Contact* contact = collData->contacts;
            contact->contactNormal = plane.plane->direction;
            contact->penetration = -ballDist;
            contact->contactPoint = ballPos - plane.plane->direction * (ballDist + capsule.capsule->radius);
            contact->setBodyData(capsule.capsule->body, nullptr, collData->friction, collData->restitution);
            collData->addContacts(1);
        }
    }

    // Lower ball
    if (collData->hasMoreContacts())
    {
        const auto ballPos = toCyclone(Vector3RotateByQuaternion(Vector3{ 0, -0.5f * capsule.capsule->height, 0 }
            , toRaylib(capsuleOrient))) + capsulePos;

        if (const float ballDist = plane.plane->direction * ballPos - capsule.capsule->radius - plane.plane->offset;
                ballDist < 0)
        {
            cyclone::Contact* contact = collData->contacts;
            contact->contactNormal = plane.plane->direction;
            contact->penetration = -ballDist;
            contact->contactPoint = ballPos - plane.plane->direction * (ballDist + capsule.capsule->radius);
            contact->setBodyData(capsule.capsule->body, nullptr, collData->friction, collData->restitution);
            collData->addContacts(1);
        }
    }
}

void CollisionBody::integrate(float dt) 
{
    getRigidBody()->integrate(dt);
    getPrimitive()->calculateInternals();
}

void CollisionSphere::collide(CollisionBox& box) 
{ 
    boxSphereCollisionImpl(box, *this, collData); 
}

void CollisionSphere::collide(CollisionSphere& sphere) 
{ 
    sphereSphereCollisionImpl(sphere, *this, collData); 
}

void CollisionSphere::collide(CollisionPlane& plane) 
{ 
    spherePlaneCollisionImpl(*this, plane, collData); 
}

void CollisionBox::collide(CollisionBox& box) 
{ 
    boxBoxCollisionImpl(box, *this, collData); 
}

void CollisionBox::collide(CollisionSphere& sphere) 
{ 
    boxSphereCollisionImpl(*this, sphere, collData); 
}

void CollisionBox::collide(CollisionPlane& plane)
{ 
    boxPlaneCollisionImpl(*this, plane, collData); 
}

void CollisionPlane::collide(CollisionBox& box) 
{ 
    boxPlaneCollisionImpl(box, *this, collData); 
}

void CollisionPlane::collide(CollisionSphere& sphere) 
{ 
    spherePlaneCollisionImpl(sphere, *this, collData); 
}

void CollisionCapsule::collide(CollisionPlane& plane)
{
    capsulePlaneCollisionImpl(*this, plane, collData);
}