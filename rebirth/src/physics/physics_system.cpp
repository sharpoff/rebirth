/*
#include <rebirth/physics/physics_system.h>

#include <rebirth/physics/physics_helpers.h>
#include <rebirth/physics/rigid_body.h>
#include <rebirth/util/logger.h>

#include <tracy/Tracy.hpp>

PhysicsSystem g_physicsSystem;

void PhysicsSystem::initialize()
{
    ZoneScopedN("Physics init");

    // Register allocation hook.
    JPH::RegisterDefaultAllocator();

    JPH::Factory::sInstance = new JPH::Factory();

    // Register all physics types with the factory and install their collision handlers with the CollisionDispatch class.
    JPH::RegisterTypes();

    // We need a temp allocator for temporary allocations during the physics update. We're
    // pre-allocating 10 MB to avoid having to do allocations during the physics update.
    tempAllocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024);

    // We need a job system that will execute physics jobs on multiple threads.
    jobSystem = new JPH::JobSystemThreadPool(
        JPH::cMaxPhysicsJobs,
        JPH::cMaxPhysicsBarriers,
        JPH::thread::hardware_concurrency() - 1);

    physicsSystem.Init(
        maxBodies,
        numBodyMutexes,
        maxBodies,
        maxContactConstraints,
        broadPhaseLayerInterface,
        objectVsBroadPhaseLayerFilter,
        objectVsObjectFilter);

    physicsSystem.SetBodyActivationListener(&bodyActivationListener);
    physicsSystem.SetContactListener(&contactListener);
}

void PhysicsSystem::shutdown()
{
    ZoneScopedN("Physics shutdown");

    JPH::BodyInterface &bodyInterface = physicsSystem.GetBodyInterface();

    for (auto &rigidBody : rigidBodies) {
        bodyInterface.RemoveBody(rigidBody.getBodyID());
        bodyInterface.DestroyBody(rigidBody.getBodyID());
    }

    JPH::UnregisterTypes();

    delete tempAllocator;
    delete jobSystem;
}

RigidBodyID PhysicsSystem::createBox(Transform transform, vec3 halfExtent, bool isStatic)
{
    JPH::BoxShapeSettings settings(MathToJolt(halfExtent));
    settings.SetEmbedded();

    JPH::Ref<JPH::Shape> shape;
    JPH::Shape::ShapeResult result = settings.Create();
    if (result.IsValid()) {
        shape = result.Get();
    } else {
        logger::logError("Failed to create physics sphere: ", result.GetError());
        return RigidBodyID::Invalid;
    }

    vec3 position = transform.getPosition();
    quat rotation = transform.getRotation();

    JPH::BodyCreationSettings bodySettings = JPH::BodyCreationSettings(
        shape.GetPtr(),
        MathToJolt(position),
        MathToJolt(rotation),
        isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic,
        isStatic ? Layers::NON_MOVING : Layers::MOVING);
    JPH::EActivation activation = isStatic ? JPH::EActivation::DontActivate : JPH::EActivation::Activate;

    return createShape(shape, bodySettings, activation, isStatic);
}

RigidBodyID PhysicsSystem::createSphere(Transform transform, float radius, bool isStatic)
{
    JPH::SphereShapeSettings settings(radius);
    settings.SetEmbedded();

    JPH::Ref<JPH::Shape> shape;
    JPH::Shape::ShapeResult result = settings.Create();
    if (result.IsValid()) {
        shape = result.Get();
    } else {
        logger::logError("Failed to create physics sphere: ", result.GetError());
        return RigidBodyID::Invalid;
    }

    vec3 position = transform.getPosition();
    quat rotation = transform.getRotation();

    JPH::BodyCreationSettings bodySettings = JPH::BodyCreationSettings(
        shape.GetPtr(),
        MathToJolt(position),
        MathToJolt(rotation),
        isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic,
        isStatic ? Layers::NON_MOVING : Layers::MOVING);
    JPH::EActivation activation = isStatic ? JPH::EActivation::DontActivate : JPH::EActivation::Activate;

    return createShape(shape, bodySettings, activation, isStatic);
}

RigidBodyID PhysicsSystem::createShape(
    JPH::Ref<JPH::Shape> &shape,
    JPH::BodyCreationSettings settings,
    JPH::EActivation activation,
    bool isStatic)
{
    JPH::BodyInterface &bodyInterface = physicsSystem.GetBodyInterface();
    RigidBody rigidBody(bodyInterface.CreateBody(settings), shape, isStatic);
    bodyInterface.AddBody(rigidBody.getBodyID(), activation);

    rigidBodies.push_back(rigidBody);
    return RigidBodyID(rigidBodies.size() - 1);
}

void PhysicsSystem::removeRigidBody(RigidBodyID id)
{
    if (id != RigidBodyID::Invalid) {
        JPH::BodyInterface &bodyInterface = physicsSystem.GetBodyInterface();
        RigidBody &rigidBody = getRigidBody(id);
        bodyInterface.RemoveBody(rigidBody.getBodyID());
        bodyInterface.DestroyBody(rigidBody.getBodyID());

        rigidBodies.erase(rigidBodies.begin() + ID(id));
    }
}

void PhysicsSystem::setFriction(RigidBodyID id, float friction)
{
    JPH::BodyInterface &bodyInterface = physicsSystem.GetBodyInterface();
    bodyInterface.SetFriction(getRigidBody(id).getBodyID(), friction);
}

void PhysicsSystem::update(float dt)
{
    ZoneScopedN("Physics update");

    physicsSystem.Update(1.0f / 60.0f, 1, tempAllocator, jobSystem);
}

vec3 PhysicsSystem::getPosition(RigidBodyID id)
{
    JPH::BodyInterface &bodyInterface = physicsSystem.GetBodyInterface();
    return JoltToMath(bodyInterface.GetPosition(getRigidBody(id).getBodyID()));
}

quat PhysicsSystem::getRotation(RigidBodyID id)
{
    JPH::BodyInterface &bodyInterface = physicsSystem.GetBodyInterface();
    return JoltToMath(bodyInterface.GetRotation(getRigidBody(id).getBodyID()));
}

RigidBody &PhysicsSystem::getRigidBody(RigidBodyID id)
{
    return rigidBodies[ID(id)];
}

void PhysicsSystem::setLinearVelocity(RigidBodyID id, vec3 velocity)
{
    JPH::BodyInterface &bodyInterface = physicsSystem.GetBodyInterface();
    bodyInterface.SetLinearVelocity(getRigidBody(id).getBodyID(), MathToJolt(velocity));
}

void PhysicsSystem::activateBody(RigidBodyID id)
{
    JPH::BodyInterface &bodyInterface = physicsSystem.GetBodyInterface();
    bodyInterface.ActivateBody(getRigidBody(id).getBodyID());
}
    */