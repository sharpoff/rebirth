#pragma once

#include <Jolt/Jolt.h>

#include <Jolt/Core/Factory.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include "Jolt/Physics/Body/BodyID.h"
#include "Jolt/Physics/Character/CharacterVirtual.h"

#include <physics/layers.h>
#include <physics/listeners.h>

class Physics
{
public:
    Physics() = default;
    Physics(Physics const &) = delete;
    void operator=(Physics const &) = delete;

    void initialize();
    void shutdown();

    void update();

    JPH::BodyID createBox(JPH::Vec3 position, JPH::Quat rotation, JPH::Vec3 halfExtent, bool isStatic);
    JPH::Ref<JPH::CharacterVirtual> createPlayer(JPH::Ref<JPH::CharacterVirtualSettings> settings, const JPH::Vec3 &position);

    void removeBodyById(JPH::BodyID id);

    JPH::BodyID rayCast(JPH::Vec3 origin, JPH::Vec3 direction);

    JPH::Vec3 getPosition(JPH::BodyID bodyId);
    JPH::Quat getRotation(JPH::BodyID bodyId);

    void setPosition(JPH::BodyID bodyId, JPH::Vec3 position);
    void setRotation(JPH::BodyID bodyId, JPH::Quat rotation);
    void setScale(JPH::BodyID bodyId, JPH::Vec3 scale);

    JPH::PhysicsSystem *getSystem() { return &physicsSystem; }
    JPH::TempAllocatorImpl *getTempAllocator() { return tempAllocator; };
    JPH::BodyInterface &getBodyInterface() { return physicsSystem.GetBodyInterface(); };

    JPH::RefConst<JPH::Shape> &getCharacterStandingShape() { return characterStandingShape; }
    JPH::RefConst<JPH::Shape> &getCharacterCrouchingShape() { return characterCrouchingShape; }
    JPH::RefConst<JPH::Shape> &getCharacterInnerCrouchingShape() { return characterInnerCrouchingShape; }
    JPH::RefConst<JPH::Shape> &getCharacterInnerStandingShape() { return characterInnerStandingShape; }

private:
    void createDefaultShapes();

    JPH::JobSystemThreadPool *jobSystem;
    JPH::TempAllocatorImpl *tempAllocator;

    // mapping table from object layer to broadphase layer
    BPLayerInterfaceImpl broadPhaseLayerInterface;

    // class that filters object vs broadphase layers
    ObjectVsBroadPhaseLayerFilterImpl objectVsBroadPhaseLayerFilter;

    // class that filters object vs object layers
    ObjectLayerPairFilterImpl mobjectVsObjectFilter;

    BodyActivationListener bodyActivationListener;
    ContactListener contactListener;

    // default shapes
    JPH::RefConst<JPH::Shape> characterStandingShape;
    JPH::RefConst<JPH::Shape> characterCrouchingShape;
    JPH::RefConst<JPH::Shape> characterInnerCrouchingShape;
    JPH::RefConst<JPH::Shape> characterInnerStandingShape;

    // character
    JPH::CharacterVsCharacterCollisionSimple characterVsCharacterCollision;
    CharacterContactListenerImpl characterContactListener;

    JPH::PhysicsSystem physicsSystem;
    const float tickDeltaFixed = 1.0f / 60.0f;
};