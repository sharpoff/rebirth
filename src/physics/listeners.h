#include <Jolt/Jolt.h>
#include "Jolt/Physics/Body/BodyActivationListener.h"
#include "Jolt/Physics/Body/BodyActivationListener.h"
#include "Jolt/Physics/Collision/ContactListener.h"
#include "Jolt/Physics/Character/CharacterVirtual.h"

const bool enableDebugOutput = false;

class ContactListener : public JPH::ContactListener
{
    virtual JPH::ValidateResult	OnContactValidate(const JPH::Body &inBody1, const JPH::Body &inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult &inCollisionResult) override
    {
        if (enableDebugOutput) {
            printf("[PHYSICS] Contact validate callback\n");
        }

        return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
    }

	virtual void OnContactAdded(const JPH::Body &inBody1, const JPH::Body &inBody2, const JPH::ContactManifold &inManifold, JPH::ContactSettings &ioSettings)  override
    {
        if (enableDebugOutput) {
            printf("[PHYSICS] Contact added\n");
        }
    }

	virtual void OnContactPersisted(const JPH::Body &inBody1, const JPH::Body &inBody2, const JPH::ContactManifold &inManifold, JPH::ContactSettings &ioSettings) override
    {
        if (enableDebugOutput) {
            printf("[PHYSICS] Contact persisted\n");
        }
    }

	virtual void OnContactRemoved(const JPH::SubShapeIDPair &inSubShapePair) override
    {
        if (enableDebugOutput) {
            printf("[PHYSICS] Contact removed\n");
        }
    }
};

class CharacterContactListenerImpl : public JPH::CharacterContactListener
{
    /// Callback to adjust the velocity of a body as seen by the character. Can be adjusted to e.g. implement a conveyor belt or an inertial dampener system of a sci-fi space ship.
    virtual void OnAdjustBodyVelocity(const JPH::CharacterVirtual *inCharacter, const JPH::Body &inBody2, JPH::Vec3 &ioLinearVelocity, JPH::Vec3 &ioAngularVelocity) override
    {
    }

    // Called whenever the character collides with a body.
    virtual void OnContactAdded(const JPH::CharacterVirtual *inCharacter, const JPH::BodyID &inBodyID2, const JPH::SubShapeID &inSubShapeID2, JPH::RVec3Arg inContactPosition, JPH::Vec3Arg inContactNormal, JPH::CharacterContactSettings &ioSettings) override
    {
    }

    // Called whenever the character persists colliding with a body.
    virtual void OnContactPersisted(const JPH::CharacterVirtual *inCharacter, const JPH::BodyID &inBodyID2, const JPH::SubShapeID &inSubShapeID2, JPH::RVec3Arg inContactPosition, JPH::Vec3Arg inContactNormal, JPH::CharacterContactSettings &ioSettings) override
    {
    }

    // Called whenever the character loses contact with a body.
    virtual void OnContactRemoved(const JPH::CharacterVirtual *inCharacter, const JPH::BodyID &inBodyID2, const JPH::SubShapeID &inSubShapeID2) override
    {
    }

    // Called whenever the character collides with a virtual character.
    virtual void OnCharacterContactAdded(const JPH::CharacterVirtual *inCharacter, const JPH::CharacterVirtual *inOtherCharacter, const JPH::SubShapeID &inSubShapeID2, JPH::RVec3Arg inContactPosition, JPH::Vec3Arg inContactNormal, JPH::CharacterContactSettings &ioSettings) override
    {
    }

    // Called whenever the character persists colliding with a virtual character.
    virtual void OnCharacterContactPersisted(const JPH::CharacterVirtual *inCharacter, const JPH::CharacterVirtual *inOtherCharacter, const JPH::SubShapeID &inSubShapeID2, JPH::RVec3Arg inContactPosition, JPH::Vec3Arg inContactNormal, JPH::CharacterContactSettings &ioSettings) override
    {
    }

    // Called whenever the character loses contact with a virtual character.
    virtual void OnCharacterContactRemoved(const JPH::CharacterVirtual *inCharacter, const JPH::CharacterID &inOtherCharacterID, const JPH::SubShapeID &inSubShapeID2) override
    {
    }

    // Called whenever the character movement is solved and a constraint is hit. Allows the listener to override the resulting character velocity (e.g. by preventing sliding along certain surfaces).
    virtual void OnContactSolve(const JPH::CharacterVirtual *inCharacter, const JPH::BodyID &inBodyID2, const JPH::SubShapeID &inSubShapeID2, JPH::RVec3Arg inContactPosition, JPH::Vec3Arg inContactNormal, JPH::Vec3Arg inContactVelocity, const JPH::PhysicsMaterial *inContactMaterial, JPH::Vec3Arg inCharacterVelocity, JPH::Vec3 &ioNewCharacterVelocity) override
    {
    }
};

class BodyActivationListener : public JPH::BodyActivationListener
{
public:
	virtual void OnBodyActivated(const JPH::BodyID &inBodyID, JPH::uint64 inBodyUserData) override
    {
        if (enableDebugOutput) {
            printf("[PHYSICS] Body activated\n");
        }
    }

	virtual void OnBodyDeactivated(const JPH::BodyID &inBodyID, JPH::uint64 inBodyUserData) override
    {
        if (enableDebugOutput) {
            printf("[PHYSICS] Body deactivated\n");
        }
    }
};