#pragma once

#include <Jolt/Jolt.h>

#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>

#include <rebirth/math/transform.h>
#include <rebirth/types/id_types.h>

#include <rebirth/physics/physics_layers.h>
#include <rebirth/physics/physics_listeners.h>
#include <rebirth/physics/rigid_body.h>

class PhysicsSystem
{
public:
    PhysicsSystem() = default;
    PhysicsSystem(PhysicsSystem const &) = delete;
    void operator=(PhysicsSystem const &) = delete;

    void initialize();
    void shutdown();

    void update(float dt);

    RigidBodyID createBox(Transform transform, vec3 halfExtent, bool isStatic = false);
    RigidBodyID createSphere(Transform transform, float radius, bool isStatic = false);
    RigidBodyID createShape(
        JPH::Ref<JPH::Shape> &shape,
        JPH::BodyCreationSettings settings,
        JPH::EActivation activation,
        bool isStatic = false);

    void removeRigidBody(RigidBodyID id);

    void setFriction(RigidBodyID id, float friction);
    void setLinearVelocity(RigidBodyID id, vec3 velocity);
    void activateBody(RigidBodyID id);

    vec3 getPosition(RigidBodyID id);
    quat getRotation(RigidBodyID id);
    JPH::PhysicsSystem &getPhysicsSystem() { return physicsSystem; };
    JPH::BodyInterface &getBodyInterface() { return physicsSystem.GetBodyInterface(); };
    RigidBody &getRigidBody(RigidBodyID id);

private:
    JPH::JobSystemThreadPool *jobSystem;
    JPH::TempAllocatorImpl *tempAllocator;

    // This is the max amount of rigid bodies that you can add to the physics system. If you try to add more you'll get an error.
    // Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
    const uint maxBodies = 1024;

    // This determines how many mutexes to allocate to protect rigid bodies from concurrent access. Set it to 0 for the default settings.
    const uint numBodyMutexes = 0;

    // This is the max amount of body pairs that can be queued at any time (the broad phase will detect overlapping
    // body pairs based on their bounding boxes and will insert them into a queue for the narrowphase).
    const uint maxContactConstraints = 1024;

    // mapping table from object layer to broadphase layer
    BPLayerInterfaceImpl broadPhaseLayerInterface;

    // class that filters object vs broadphase layers
    ObjectVsBroadPhaseLayerFilterImpl objectVsBroadPhaseLayerFilter;

    // class that filters object vs object layers
    ObjectLayerPairFilterImpl objectVsObjectFilter;

    BodyActivationListener bodyActivationListener;
    ContactListener contactListener;

    JPH::PhysicsSystem physicsSystem;

    std::vector<RigidBody> rigidBodies;
};

extern PhysicsSystem g_physicsSystem;