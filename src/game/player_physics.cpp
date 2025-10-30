#include "game/player.h"

#include "Jolt/Geometry/Plane.h"
#include "Jolt/Physics/PhysicsSystem.h"

#include "physics/constants.h"
#include "physics/helpers.h"
#include "physics/layers.h"

void Player::initializePhysics()
{
    JPH::Ref<JPH::CharacterVirtualSettings> settings = new JPH::CharacterVirtualSettings();
    settings->mMaxSlopeAngle = sMaxSlopeAngle;
	settings->mMaxStrength = sMaxStrength;
	settings->mShape = physics->getCharacterStandingShape();
	settings->mBackFaceMode = sBackFaceMode;
	settings->mCharacterPadding = sCharacterPadding;
	settings->mPenetrationRecoverySpeed = sPenetrationRecoverySpeed;
	settings->mPredictiveContactDistance = sPredictiveContactDistance;
	settings->mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -kCharacterRadiusStanding); // Accept contacts that touch the lower sphere of the capsule
	settings->mEnhancedInternalEdgeRemoval = sEnhancedInternalEdgeRemoval;
	settings->mInnerBodyShape = sCreateInnerBody ? physics->getCharacterInnerStandingShape() : nullptr;
	settings->mInnerBodyLayer = Layers::MOVING;

    character = physics->createPlayer(settings, JPH::Vec3(0.0f, 10.0f, 3.0f));
	character->SetLinearVelocity(physics->getSystem()->GetGravity());
}

void Player::updatePhysics(float deltaTime)
{
    if (!character) return;

	if (canMove)
		updatePhysicsController(deltaTime);

    // Settings for our update function
    JPH::CharacterVirtual::ExtendedUpdateSettings settings{};
    if (!sEnableStickToFloor)
        settings.mStickToFloorStepDown = JPH::Vec3::sZero();
    else
        settings.mStickToFloorStepDown = -character->GetUp() * settings.mStickToFloorStepDown.Length();

    if (!sEnableWalkStairs)
        settings.mWalkStairsStepUp = JPH::Vec3::sZero();
    else
        settings.mWalkStairsStepUp = character->GetUp() * settings.mWalkStairsStepUp.Length();

    JPH::PhysicsSystem *physicsSystem = physics->getSystem();

    // Update the character position
    character->ExtendedUpdate(deltaTime,
        -character->GetUp() * physicsSystem->GetGravity().Length(),
        settings,
        physicsSystem->GetDefaultBroadPhaseLayerFilter(Layers::MOVING),
        physicsSystem->GetDefaultLayerFilter(Layers::MOVING),
        {},
        {},
        *physics->getTempAllocator());

    worldTransform = character->GetWorldTransform();
}

void Player::updatePhysicsController(float deltaTime)
{
    JPH::PhysicsSystem *physicsSystem = physics->getSystem();

    bool playerControlsHorizontalVelocity = sControlMovementDuringJump || character->IsSupported();
	if (playerControlsHorizontalVelocity) {
		// Smooth the player input
		desiredVelocity = sEnableCharacterInertia? 0.25f * moveDir * sCharacterSpeed + 0.75f * desiredVelocity : moveDir * sCharacterSpeed;

		// True if the player intended to move
		allowSliding = !moveDir.IsNearZero();
	}
	else {
		// While in air we allow sliding
		allowSliding = true;
	}

	// Update the character rotation and its up vector to match the up vector set by the user settings
	JPH::Quat characterUpRotation = MathToJolt(camera.getRotation()) * JPH::Quat::sEulerAngles(JPH::Vec3(sUpRotationX, 0, sUpRotationZ));
	character->SetUp(characterUpRotation.RotateAxisY());
	character->SetRotation(characterUpRotation);

	// A cheaper way to update the character's ground velocity,
	// the platforms that the character is standing on may have changed velocity
	character->UpdateGroundVelocity();

	// Determine new basic velocity
	JPH::Vec3 currentVerticalVelocity = character->GetLinearVelocity().Dot(character->GetUp()) * character->GetUp();
	JPH::Vec3 groundVelocity = character->GetGroundVelocity();
	JPH::Vec3 newVelocity;
	bool movingTowardsGround = (currentVerticalVelocity.GetY() - groundVelocity.GetY()) < 0.1f;
	if (character->GetGroundState() == JPH::CharacterVirtual::EGroundState::OnGround	// If on ground
		&& (sEnableCharacterInertia?
			movingTowardsGround													// Inertia enabled: And not moving away from ground
			: !character->IsSlopeTooSteep(character->GetGroundNormal())))			// Inertia disabled: And not on a slope that is too steep
	{
		// Assume velocity of ground when on ground
		newVelocity = groundVelocity;

		// Jump
		if (jumping && movingTowardsGround)
			newVelocity += sJumpSpeed * character->GetUp();
	}
	else
		newVelocity = currentVerticalVelocity;

	// Gravity
	newVelocity += (characterUpRotation * physicsSystem->GetGravity()) * deltaTime;

	if (playerControlsHorizontalVelocity) {
		// Player input
		newVelocity += characterUpRotation * desiredVelocity;
	}
	else {
		// Preserve horizontal velocity
		JPH::Vec3 currentHorizontalVelocity = character->GetLinearVelocity() - currentVerticalVelocity;
		newVelocity += currentHorizontalVelocity;
	}

	// Update character velocity
	character->SetLinearVelocity(newVelocity);
}