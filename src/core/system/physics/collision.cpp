#include "collision.h"

// Collision implementation
static void boxBoxCollisionImpl(CollisionBox& box1, CollisionBox& box2, cyclone::CollisionData* collData);
static void boxSphereCollisionImpl(CollisionBox& box, CollisionSphere& sphere, cyclone::CollisionData* collData);
static void boxPlaneCollisionImpl(CollisionBox& box, CollisionPlane& plane, cyclone::CollisionData* collData);
static void sphereSphereCollisionImpl(CollisionSphere& sphere1, CollisionSphere& sphere2, cyclone::CollisionData* collData);
static void spherePlaneCollisionImpl(CollisionSphere& sphere, CollisionPlane& plane, cyclone::CollisionData* collData);

void CollisionSphere::collide(CollisionBox& box) {
    boxSphereCollisionImpl(box, *this, collData);
}

void CollisionSphere::collide(CollisionSphere& sphere) {
    sphereSphereCollisionImpl(sphere, *this, collData);
}

void CollisionSphere::collide(CollisionPlane& plane) {
    spherePlaneCollisionImpl(*this, plane, collData);
}

void CollisionBox::collide(CollisionBox& box) {
    boxBoxCollisionImpl(box, *this, collData);
}

void CollisionBox::collide(CollisionSphere& sphere) {
    boxSphereCollisionImpl(*this, sphere, collData);
}

void CollisionBox::collide(CollisionPlane& plane) {
    boxPlaneCollisionImpl(*this, plane, collData);
}

void CollisionPlane::collide(CollisionBox& box) {
    boxPlaneCollisionImpl(box, *this, collData);
}

void CollisionPlane::collide(CollisionSphere& sphere) {
    spherePlaneCollisionImpl(sphere, *this, collData);
}

void CollisionPlane::collide(CollisionPlane& plane) {
}

void boxBoxCollisionImpl(CollisionBox& box1, CollisionBox& box2, cyclone::CollisionData* collData) {
    if (&box1 != &box2 && collData->hasMoreContacts()) {
        cyclone::CollisionDetector::boxAndBox(static_cast<cyclone::CollisionBox&>(box1), static_cast<cyclone::CollisionBox&>(box2), collData);
    }
}

void boxSphereCollisionImpl(CollisionBox& box, CollisionSphere& sphere, cyclone::CollisionData* collData) {
    if (collData->hasMoreContacts()) {
        cyclone::CollisionDetector::boxAndSphere(static_cast<cyclone::CollisionBox&>(box), static_cast<cyclone::CollisionSphere&>(sphere), collData);
    }
}

void boxPlaneCollisionImpl(CollisionBox& box, CollisionPlane& plane, cyclone::CollisionData* collData) {
    if (collData->hasMoreContacts()) {
        cyclone::CollisionDetector::boxAndHalfSpace(static_cast<cyclone::CollisionBox&>(box), static_cast<cyclone::CollisionPlane&>(plane), collData);
    }
}

void sphereSphereCollisionImpl(CollisionSphere& sphere1, CollisionSphere& sphere2, cyclone::CollisionData* collData) {
    if (&sphere1 != &sphere2 && collData->hasMoreContacts()) {
        cyclone::CollisionDetector::sphereAndSphere(static_cast<cyclone::CollisionSphere&>(sphere1), static_cast<cyclone::CollisionSphere&>(sphere2), collData);
    }
}

void spherePlaneCollisionImpl(CollisionSphere& sphere, CollisionPlane& plane, cyclone::CollisionData* collData) {
    if (collData->hasMoreContacts()) {
        cyclone::CollisionDetector::sphereAndHalfSpace(static_cast<cyclone::CollisionSphere&>(sphere), static_cast<cyclone::CollisionPlane&>(plane), collData);
    }
}