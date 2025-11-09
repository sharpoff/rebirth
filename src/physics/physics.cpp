#include "physics/physics.h"

#include "Jolt/Physics/Body/BodyID.h"
#include "Jolt/Physics/Body/BodyInterface.h"
#include "Jolt/Physics/Collision/BroadPhase/BroadPhaseQuery.h"
#include "Jolt/Physics/Collision/RayCast.h"
#include "Jolt/Physics/Collision/CastResult.h" // IWYU pragma: export
#include "Jolt/Physics/Collision/CollisionCollectorImpl.h"
#include "Jolt/Physics/Character/CharacterVirtual.h"

#include "physics/constants.h"

#include "util/logger.h"

#include "tracy/Tracy.hpp"

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
    jobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, JPH::thread::hardware_concurrency() - 1);

    physicsSystem.Init(physics::maxBodies, physics::numBodyMutexes, physics::maxBodies, physics::maxContactConstraints, broadPhaseLayerInterface, objectVsBroadPhaseLayerFilter, mobjectVsObjectFilter);

    physicsSystem.SetBodyActivationListener(&bodyActivationListener);
    physicsSystem.SetContactListener(&contactListener);
}

void Physics::shutdown()
{
    ZoneScopedN("Physics shutdown");

    JPH::UnregisterTypes();

    delete tempAllocator;
    delete jobSystem;

    LOGI("%s", "Physics shutdown");
}

void Physics::update()
{
    ZoneScopedN("Physics update");

    physicsSystem.Update(tickDeltaFixed, 1, tempAllocator, jobSystem);
}

JPH::BodyID Physics::createBox(JPH::Vec3 position, JPH::Quat rotation, JPH::Vec3 halfExtent, bool isStatic)
{
    JPH::BoxShapeSettings settings(halfExtent);
    settings.SetEmbedded();

    JPH::Ref<JPH::Shape> shape;
    JPH::Shape::ShapeResult result = settings.Create();
    if (result.IsValid()) {
        shape = result.Get();
    } else {
        LOGE("Failed to create physics box: %s", result.GetError().c_str());
        return JPH::BodyID(); // invalid
    }

    JPH::EMotionType motionType = isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic;
    JPH::ObjectLayer layer = isStatic ? Layers::NON_MOVING : Layers::MOVING;
    JPH::EActivation activation = isStatic ? JPH::EActivation::DontActivate : JPH::EActivation::Activate;

    JPH::BodyCreationSettings bodyCreateInfo = JPH::BodyCreationSettings(
        shape.GetPtr(),
        position, rotation.Normalized(),
        motionType, layer
    );

    JPH::BodyInterface &bodyInterface = getBodyInterface();
    JPH::BodyID bodyId = bodyInterface.CreateAndAddBody(bodyCreateInfo, activation);
    if (bodyId.IsInvalid()) {
        LOGE("%s", "Cannot create physics body, out of bodies!");
        return JPH::BodyID(); // invalid
    }

    bodyInterface.SetLinearVelocity(bodyId, physicsSystem.GetGravity());

    return bodyId;
}

JPH::Ref<JPH::CharacterVirtual> Physics::createPlayer(JPH::Ref<JPH::CharacterVirtualSettings> settings, const JPH::Vec3 &position)
{
    JPH::Ref<JPH::CharacterVirtual> character = new JPH::CharacterVirtual(settings, position, JPH::Quat::sIdentity(), 0, &physicsSystem);
    character->SetCharacterVsCharacterCollision(&characterVsCharacterCollision);
	characterVsCharacterCollision.Add(character);

	// Install contact listener for all characters
	for (JPH::CharacterVirtual *character : characterVsCharacterCollision.mCharacters)
		character->SetListener(&characterContactListener);

    return character;
}

void Physics::removeBodyById(JPH::BodyID id)
{
    JPH::BodyInterface &bodyInterface = getBodyInterface();
    bodyInterface.RemoveBody(id);
    bodyInterface.DestroyBody(id);
}

JPH::Vec3 Physics::getPosition(JPH::BodyID bodyId)
{
    assert(bodyId != JPH::BodyID());

    JPH::BodyInterface &bodyInterface = getBodyInterface();
    return bodyInterface.GetPosition(bodyId);
}

JPH::Quat Physics::getRotation(JPH::BodyID bodyId)
{
    assert(bodyId != JPH::BodyID());

    JPH::BodyInterface &bodyInterface = getBodyInterface();
    return bodyInterface.GetRotation(bodyId);
}

void Physics::setPosition(JPH::BodyID bodyId, JPH::Vec3 position)
{
    assert(bodyId != JPH::BodyID());

    JPH::BodyInterface &bodyInterface = getBodyInterface();
    bodyInterface.SetPosition(bodyId, position, JPH::EActivation::Activate);
}

void Physics::setRotation(JPH::BodyID bodyId, JPH::Quat rotation)
{
    assert(bodyId != JPH::BodyID());

    JPH::BodyInterface &bodyInterface = getBodyInterface();
    bodyInterface.SetRotation(bodyId, rotation, JPH::EActivation::Activate);
}

void Physics::setScale(JPH::BodyID bodyId, JPH::Vec3 scale)
{
    JPH::BodyInterface &bodyInterface = getBodyInterface();
    JPH::RefConst<JPH::Shape> shape = bodyInterface.GetShape(bodyId);
    shape->ScaleShape(scale);
    bodyInterface.SetShape(bodyId, shape, true, JPH::EActivation::Activate);
}

JPH::BodyID Physics::rayCast(JPH::Vec3 origin, JPH::Vec3 direction)
{
    JPH::RayCast ray(origin, direction);
    JPH::ClosestHitCollisionCollector<JPH::RayCastBodyCollector> collector;

    physicsSystem.GetBroadPhaseQuery().CastRay(ray, collector);

    if (collector.HadHit()) {
        LOGI("%s", "Ray cast HadHit()");
        const JPH::RayCastBodyCollector::ResultType hit = collector.mHit;

        return hit.mBodyID;
    }

    return JPH::BodyID();
}