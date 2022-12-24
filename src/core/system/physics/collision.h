#pragma once

#include "cyclone/collide_fine.h"

namespace cyclone {
    struct CollisionData;
}

class CollisionShape
{
public:
    virtual void collide(CollisionShape&) = 0;
    virtual void collide(class CollisionBox&) = 0;
    virtual void collide(class CollisionSphere&) = 0;
    virtual void collide(class CollisionPlane&) = 0;

    cyclone::CollisionData* collData;
};

class CollisionSphere : public CollisionShape, public cyclone::CollisionSphere
{
public:
    void collide(CollisionShape& shape) override {
        shape.collide(*this);
    }

    void collide(CollisionBox& box) override;
    void collide(CollisionSphere& sphere) override;
    void collide(CollisionPlane& plane) override;
};

class CollisionBox : public CollisionShape, public cyclone::CollisionBox
{
public:
    void collide(CollisionShape& shape) override {
        shape.collide(*this);
    }

    void collide(CollisionBox& box) override;
    void collide(CollisionSphere& sphere) override;
    void collide(CollisionPlane& plane) override;
};

class CollisionPlane : public CollisionShape, public cyclone::CollisionPlane
{
public:
    void collide(CollisionShape& shape) override {
        shape.collide(*this);
    }

    void collide(CollisionBox& box) override;
    void collide(CollisionSphere& sphere) override;
    void collide(CollisionPlane& plane) override;
};