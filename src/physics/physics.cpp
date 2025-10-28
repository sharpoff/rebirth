#include "physics/physics.h"

#include "Jolt/Physics/Body/BodyID.h"
#include "Jolt/Physics/Body/BodyInterface.h"
#include "Jolt/Physics/Collision/BroadPhase/BroadPhaseQuery.h"
#include "Jolt/Physics/Collision/RayCast.h"
#include "Jolt/Physics/Collision/CastResult.h" // IWYU pragma: export
#include "Jolt/Physics/Collision/CollisionCollectorImpl.h"
#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"
#include "Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h"
#include "Jolt/Physics/Character/CharacterVirtual.h"

#include "physics/helpers.h"
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
    jobSystem = new JPH::JobSystemThreadPool(
        JPH::cMaxPhysicsJobs,
        JPH::cMaxPhysicsBarriers,
        JPH::thread::hardware_concurrency() - 1);

    physicsSystem_.Init(
        maxBodies,
        numBodyMutexes,
        maxBodies,
        maxContactConstraints,
        broadPhaseLayerInterface,
        objectVsBroadPhaseLayerFilter,
        objectVsObjectFilter);

    physicsSystem_.SetBodyActivationListener(&bodyActivationListener);
    physicsSystem_.SetContactListener(&contactListener);

    createDefaultShapes();

    logger::logInfo("Physics initialized");
}

void Physics::shutdown()
{
    ZoneScopedN("Physics shutdown");

    JPH::BodyInterface &bodyInterface = physicsSystem_.GetBodyInterface();

    for (auto &body : bodies) {
        bodyInterface.RemoveBody(body);
        bodyInterface.DestroyBody(body);
    }

    JPH::UnregisterTypes();

    delete tempAllocator;
    delete jobSystem;

    logger::logInfo("Physics shutdown");
}

void Physics::preUpdate(float dt)
{
}

void Physics::update()
{
    ZoneScopedN("Physics update");

    physicsSystem_.Update(tickDelta, 1, tempAllocator, jobSystem);
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
        logger::logError("Failed to create physics box: ", result.GetError());
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

    JPH::BodyInterface &bodyInterface = physicsSystem_.GetBodyInterface();
    JPH::BodyID bodyId = bodyInterface.CreateAndAddBody(bodyCreateInfo, activation);
    if (bodyId.IsInvalid()) {
        logger::logError("Cannot create physics body - out of bodies!");
        return JPH::BodyID(); // invalid
    }

    bodies.push_back(bodyId);

    return bodyId;
}

JPH::Ref<JPH::CharacterVirtual> Physics::createPlayer(JPH::Ref<JPH::CharacterVirtualSettings> settings)
{
    JPH::Ref<JPH::CharacterVirtual> character = new JPH::CharacterVirtual(settings, JPH::RVec3::sZero(), JPH::Quat::sIdentity(), 0, &physicsSystem_);
    character->SetCharacterVsCharacterCollision(&characterVsCharacterCollision_);
	characterVsCharacterCollision_.Add(character);

	// Install contact listener for all characters
	for (JPH::CharacterVirtual *character : characterVsCharacterCollision_.mCharacters)
		character->SetListener(&characterContactListener_);

    return character;
}

vec3 Physics::getPosition(JPH::BodyID bodyId)
{
    assert(bodyId != JPH::BodyID());

    JPH::BodyInterface &bodyInterface = physicsSystem_.GetBodyInterface();
    return JoltToMath(bodyInterface.GetPosition(bodyId));
}

quat Physics::getRotation(JPH::BodyID bodyId)
{
    assert(bodyId != JPH::BodyID());

    JPH::BodyInterface &bodyInterface = physicsSystem_.GetBodyInterface();
    return JoltToMath(bodyInterface.GetRotation(bodyId));
}

void Physics::setPosition(JPH::BodyID bodyId, vec3 position)
{
    assert(bodyId != JPH::BodyID());

    JPH::BodyInterface &bodyInterface = physicsSystem_.GetBodyInterface();
    bodyInterface.SetPosition(bodyId, MathToJolt(position), JPH::EActivation::Activate);
}

void Physics::setRotation(JPH::BodyID bodyId, quat rotation)
{
    assert(bodyId != JPH::BodyID());

    JPH::BodyInterface &bodyInterface = physicsSystem_.GetBodyInterface();
    bodyInterface.SetRotation(bodyId, MathToJolt(rotation), JPH::EActivation::Activate);
}

JPH::BodyID Physics::rayCast(vec3 origin, vec3 direction)
{
    JPH::RayCast ray(MathToJolt(origin), MathToJolt(direction));
    JPH::ClosestHitCollisionCollector<JPH::RayCastBodyCollector> collector;

    physicsSystem_.GetBroadPhaseQuery().CastRay(ray, collector);

    if (collector.HadHit()) {
        logger::logInfo("Ray cast HadHit()");
        const JPH::RayCastBodyCollector::ResultType hit = collector.mHit;

        return hit.mBodyID;
    }

    return JPH::BodyID();
}

void Physics::createDefaultShapes()
{
    shapes["character_standing"] = JPH::RotatedTranslatedShapeSettings(JPH::Vec3(0, 0.5f * kCharacterHeightStanding + kCharacterRadiusStanding, 0), JPH::Quat::sIdentity(), new JPH::CapsuleShape(0.5f * kCharacterHeightStanding, kCharacterRadiusStanding)).Create().Get();
    shapes["character_crouching"] = JPH::RotatedTranslatedShapeSettings(JPH::Vec3(0, 0.5f * kCharacterHeightCrouching + kCharacterRadiusCrouching, 0), JPH::Quat::sIdentity(), new JPH::CapsuleShape(0.5f * kCharacterHeightCrouching, kCharacterRadiusCrouching)).Create().Get();
    shapes["character_inner_standing"] = JPH::RotatedTranslatedShapeSettings(JPH::Vec3(0, 0.5f * kCharacterHeightStanding + kCharacterRadiusStanding, 0), JPH::Quat::sIdentity(), new JPH::CapsuleShape(0.5f * kInnerShapeFraction * kCharacterHeightStanding, kInnerShapeFraction * kCharacterRadiusStanding)).Create().Get();
    shapes["character_inner_crouching"] = JPH::RotatedTranslatedShapeSettings(JPH::Vec3(0, 0.5f * kCharacterHeightCrouching + kCharacterRadiusCrouching, 0), JPH::Quat::sIdentity(), new JPH::CapsuleShape(0.5f * kInnerShapeFraction * kCharacterHeightCrouching, kInnerShapeFraction * kCharacterRadiusCrouching)).Create().Get();
}

JPH::RefConst<JPH::Shape> Physics::getShapeByName(eastl::string name)
{
    return shapes[name];
}

void Physics::updateCharacter(float deltaTime, JPH::Ref<JPH::CharacterVirtual> character, const JPH::CharacterVirtual::ExtendedUpdateSettings &settings)
{
    bool sEnableStickToFloor = true;
    bool sEnableWalkStairs = true;

	character->ExtendedUpdate(deltaTime,
		-character->GetUp() * physicsSystem_.GetGravity().Length(),
		settings,
		physicsSystem_.GetDefaultBroadPhaseLayerFilter(Layers::MOVING),
		physicsSystem_.GetDefaultLayerFilter(Layers::MOVING),
		{},
		{},
		*tempAllocator);

	vec3 oldPosition = JoltToMath(character->GetPosition());

	vec3 centerOfMass = JoltToMath(character->GetCenterOfMassPosition());
	mat4 worldTransform = JoltToMath(character->GetWorldTransform());

	JPH::CharacterVirtual::ExtendedUpdateSettings updateSettings;
	if (!sEnableStickToFloor)
		updateSettings.mStickToFloorStepDown = JPH::Vec3::sZero();
	else
		updateSettings.mStickToFloorStepDown = -character->GetUp() * updateSettings.mStickToFloorStepDown.Length();

	if (!sEnableWalkStairs)
		updateSettings.mWalkStairsStepUp = JPH::Vec3::sZero();
	else
		updateSettings.mWalkStairsStepUp = character->GetUp() * updateSettings.mWalkStairsStepUp.Length();

    // Calculate effective velocity
	vec3 newPosition = JoltToMath(character->GetPosition());
	vec3 velocity = vec3(newPosition - oldPosition) / deltaTime;
}