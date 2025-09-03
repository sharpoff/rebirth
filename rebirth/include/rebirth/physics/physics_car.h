#pragma once

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Vehicle/VehicleConstraint.h>

#include <rebirth/physics/physics_system.h>
#include <rebirth/types/types.h>

namespace rebirth
{

class PhysicsCar
{
public:
    void initialize(rebirth::PhysicsSystem &physicsSystem, vec3 position);

    void update(float deltaTime);
    void processInput();

    Transform getWheelTransform(int wheel);
    const vec3 getPosition() const;
    const quat getRotation() const;

    void setPosition(vec3 position);
    void setRotation(quat rotation);

private:
    float getWheelRadius(int wheel);
    float getWheelWidth(int wheel);

    float sInitialRollAngle = 0;
    float sMaxRollAngle = JPH::DegreesToRadians(60.0f);
    float sMaxSteeringAngle = JPH::DegreesToRadians(30.0f);
    bool sFourWheelDrive = true;
    bool sAntiRollbar = true;
    bool sLimitedSlipDifferentials = true;
    float sMaxEngineTorque = 500.0f;
    float sClutchStrength = 10.0f;
    float sFrontCasterAngle = 0.0f;
    float sFrontKingPinAngle = 0.0f;
    float sFrontCamber = 0.0f;
    float sFrontToe = 0.0f;
    float sFrontSuspensionForwardAngle = 0.0f;
    float sFrontSuspensionSidewaysAngle = 0.0f;
    float sFrontSuspensionMinLength = 0.3f;
    float sFrontSuspensionMaxLength = 0.5f;
    float sFrontSuspensionFrequency = 1.5f;
    float sFrontSuspensionDamping = 0.5f;
    float sRearSuspensionForwardAngle = 0.0f;
    float sRearSuspensionSidewaysAngle = 0.0f;
    float sRearCasterAngle = 0.0f;
    float sRearKingPinAngle = 0.0f;
    float sRearCamber = 0.0f;
    float sRearToe = 0.0f;
    float sRearSuspensionMinLength = 0.3f;
    float sRearSuspensionMaxLength = 0.5f;
    float sRearSuspensionFrequency = 1.5f;
    float sRearSuspensionDamping = 0.5f;

    RigidBodyID rigidBodyId;
    JPH::Body *mCarBody;
    JPH::BodyInterface *bodyInterface;

    JPH::Ref<JPH::VehicleConstraint> mVehicleConstraint;
    JPH::Ref<JPH::VehicleCollisionTester> mCollisionTester;

    // Player input
    float mForward = 0.0f;
    float mPreviousForward = 1.0f;
    float mRight = 0.0f;
    float mBrake = 0.0f;
    float mHandBrake = 0.0f;
};

} // namespace rebirth