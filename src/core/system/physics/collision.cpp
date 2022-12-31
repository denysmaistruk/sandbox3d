#include "collision.h"

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

void CollisionPlane::collide(CollisionPlane& plane) 
{ 
}