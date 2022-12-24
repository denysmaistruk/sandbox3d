#pragma once

#include "cyclone/collide_fine.h"

class CollisionBody
{
public:
    virtual void collide(CollisionBody& /*body*/) = 0;
    virtual void collide(class CollisionBox& /*box*/) = 0;
    virtual void collide(class CollisionSphere& /*sphere*/) = 0;
    virtual void collide(class CollisionPlane& /*plane*/) = 0;

    virtual cyclone::CollisionPrimitive* getPrimitive() { return nullptr; }

    void integrate(float dt);
    
    cyclone::Matrix4 getTransform() { return getPrimitive()->getTransform(); }
    cyclone::RigidBody* getRigidBody() { return isDynamic() ? getPrimitive()->body : nullptr; }

    bool isDynamic() { return getPrimitive() != nullptr; }
    bool isSleeping() { return isDynamic() && !getRigidBody()->getAwake(); }
       
    cyclone::CollisionData* collData;
};

class CollisionSphere : public CollisionBody
{
public:
    void collide(CollisionBody& body) override {
        body.collide(*this);
    }

    void collide(CollisionBox& box) override;
    void collide(CollisionSphere& sphere) override;
    void collide(CollisionPlane& plane) override;
    
    cyclone::CollisionPrimitive* getPrimitive() override { return sphere; }

    cyclone::CollisionSphere* sphere;
};

class CollisionBox : public CollisionBody
{
public:
    void collide(CollisionBody& body) override {
        body.collide(*this);
    }

    void collide(CollisionBox& box) override;
    void collide(CollisionSphere& sphere) override;
    void collide(CollisionPlane& plane) override;
    
    cyclone::CollisionPrimitive* getPrimitive() override { return box; }

    cyclone::CollisionBox* box;
};

class CollisionPlane : public CollisionBody
{
public:
    void collide(CollisionBody& body) override {
        body.collide(*this);
    }

    void collide(CollisionBox& box) override;
    void collide(CollisionSphere& sphere) override;
    void collide(CollisionPlane& plane) override;

    cyclone::CollisionPlane* plane;
};