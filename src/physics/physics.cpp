#include "physics/physics.h"

#include "Jolt/Physics/Body/BodyID.h"
#include "Jolt/Physics/Body/BodyInterface.h"
#include "Jolt/Physics/Collision/BroadPhase/BroadPhaseQuery.h"
#include "Jolt/Physics/Collision/RayCast.h"
#include "Jolt/Physics/Collision/CastResult.h" // IWYU pragma: export
#include "Jolt/Physics/Collision/CollisionCollectorImpl.h"

#include "game/entity.h"
#include "physics/physics_helpers.h"
#include "util/logger.h"

#include <tracy/Tracy.hpp>

void Physics::initialize()
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

    logger::logInfo("Physics initialized");
}

void Physics::shutdown()
{
    ZoneScopedN("Physics shutdown");

    JPH::BodyInterface &bodyInterface = physicsSystem.GetBodyInterface();

    for (auto &body : bodies) {
        bodyInterface.RemoveBody(body);
        bodyInterface.DestroyBody(body);
    }

    JPH::UnregisterTypes();

    delete tempAllocator;
    delete jobSystem;

    logger::logInfo("Physics shutdown");
}

void Physics::update(float dt)
{
    ZoneScopedN("Physics update");

    physicsSystem.Update(tickDelta, 1, tempAllocator, jobSystem);
}

JPH::BodyID Physics::createBox(const Entity &entity)
{
    return createBox(entity.position, entity.rotation, entity.bounds.extents, entity.isStatic);
}

JPH::BodyID Physics::createBox(vec3 position, quat rotation, vec3 halfExtent, bool isStatic)
{
    JPH::BoxShapeSettings settings(MathToJolt(halfExtent));
    settings.SetEmbedded();

    JPH::Ref<JPH::Shape> shape;
    JPH::Shape::ShapeResult result = settings.Create();
    if (result.IsValid()) {
        shape = result.Get();
    } else {
        logger::logError("Failed to create physics sphere: ", result.GetError());
        return JPH::BodyID(); // invalid
    }

    JPH::EMotionType motionType = isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic;
    JPH::ObjectLayer layer = isStatic ? Layers::NON_MOVING : Layers::MOVING;
    JPH::EActivation activation = isStatic ? JPH::EActivation::DontActivate : JPH::EActivation::Activate;

    JPH::BodyCreationSettings bodyCreateInfo = JPH::BodyCreationSettings(
        shape.GetPtr(),
        MathToJolt(position), MathToJolt(glm::normalize(rotation)),
        motionType, layer
    );

    JPH::BodyInterface &bodyInterface = physicsSystem.GetBodyInterface();
    JPH::BodyID bodyId = bodyInterface.CreateAndAddBody(bodyCreateInfo, activation);
    if (bodyId.IsInvalid()) {
        logger::logError("Cannot create physics body - out of bodies!");
        return JPH::BodyID(); // invalid
    }

    bodies.push_back(bodyId);

    return bodyId;
}

vec3 Physics::getPosition(JPH::BodyID bodyId)
{
    JPH::BodyInterface &bodyInterface = physicsSystem.GetBodyInterface();
    return JoltToMath(bodyInterface.GetPosition(bodyId));
}

quat Physics::getRotation(JPH::BodyID bodyId)
{
    JPH::BodyInterface &bodyInterface = physicsSystem.GetBodyInterface();
    return JoltToMath(bodyInterface.GetRotation(bodyId));
}

void Physics::setPosition(JPH::BodyID bodyId, vec3 position)
{
    JPH::BodyInterface &bodyInterface = physicsSystem.GetBodyInterface();
    bodyInterface.SetPosition(bodyId, MathToJolt(position), JPH::EActivation::Activate);
}

void Physics::setRotation(JPH::BodyID bodyId, quat rotation)
{
    JPH::BodyInterface &bodyInterface = physicsSystem.GetBodyInterface();
    bodyInterface.SetRotation(bodyId, MathToJolt(rotation), JPH::EActivation::Activate);
}

JPH::BodyID Physics::rayCast(vec3 origin, vec3 direction)
{
    JPH::RayCast ray(MathToJolt(origin), MathToJolt(direction));
    JPH::ClosestHitCollisionCollector<JPH::RayCastBodyCollector> collector;

    physicsSystem.GetBroadPhaseQuery().CastRay(ray, collector);

    if (collector.HadHit()) {
        logger::logInfo("Ray cast HadHit()");
        const JPH::RayCastBodyCollector::ResultType hit = collector.mHit;

        return hit.mBodyID;
    }

    return JPH::BodyID();
}