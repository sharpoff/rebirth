#include <rebirth/physics/rigid_body.h>

#include <rebirth/physics/physics_system.h>
#include <rebirth/physics/physics_helpers.h>

void RigidBody::setPosition(vec3 position)
{
    auto &bodyInterface = g_physicsSystem.getBodyInterface();
    bodyInterface.SetPosition(getBodyID(), MathToJolt(position), JPH::EActivation::Activate);
}

void RigidBody::setRotation(quat rotation)
{
    auto &bodyInterface = g_physicsSystem.getBodyInterface();
    bodyInterface.SetRotation(getBodyID(), MathToJolt(rotation), JPH::EActivation::Activate);
}

vec3 RigidBody::getPosition()
{
    auto &bodyInterface = g_physicsSystem.getBodyInterface();
    return JoltToMath(bodyInterface.GetPosition(getBodyID()));
}

quat RigidBody::getRotation()
{
    auto &bodyInterface = g_physicsSystem.getBodyInterface();
    return JoltToMath(bodyInterface.GetRotation(getBodyID()));
}

Transform RigidBody::getTransform()
{
    return Transform(getPosition(), getRotation());
}