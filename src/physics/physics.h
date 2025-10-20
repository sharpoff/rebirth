#pragma once

#include "EASTL/vector.h"
#include "math/math.h"

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

#include <physics/physics_layers.h>
#include <physics/physics_listeners.h>

class Entity;

class Physics
{
public:
    Physics() = default;
    Physics(Physics const &) = delete;
    void operator=(Physics const &) = delete;

    void initialize();
    void shutdown();

    void update(float dt);

    JPH::BodyID createBox(const Entity &entity);
    JPH::BodyID createBox(vec3 position, quat rotation, vec3 halfExtent, bool isStatic);

    vec3 getPosition(JPH::BodyID bodyId);
    quat getRotation(JPH::BodyID bodyId);

    void setPosition(JPH::BodyID bodyId, vec3 position);
    void setRotation(JPH::BodyID bodyId, quat rotation);

    JPH::BodyID rayCast(vec3 origin, vec3 direction);
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

    eastl::vector<JPH::BodyID> bodies;

    const float tickDelta = 1.0f / 60.0f;
};