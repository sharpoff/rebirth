#pragma once

#include <rebirth/math/transform.h>

#include <Jolt/Jolt.h>

#include <Jolt/Core/Reference.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>

struct RigidBody
{
    RigidBody(JPH::Body *body, JPH::Ref<JPH::Shape> shape, bool isStatic)
        : body(body), shape(shape), _static(isStatic) {};

    void setPosition(vec3 position);
    void setRotation(quat rotation);

    vec3 getPosition();
    quat getRotation();
    Transform getTransform();

    JPH::BodyID getBodyID() { return body->GetID(); };
    JPH::Body *getBody() { return body; };
    bool isStatic() const { return _static; };

private:
    JPH::Body *body;
    JPH::Ref<JPH::Shape> shape;
    bool _static = false;
};