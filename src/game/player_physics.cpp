#include "game/player.h"

#include "Jolt/Geometry/Plane.h"
#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"
#include "Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h"

#include "physics/helpers.h"
#include "physics/layers.h"

void Player::initializePhysics()
{
    standingShape = JPH::RotatedTranslatedShapeSettings(JPH::Vec3(0, 0.5f * characterHeightStanding + characterRadiusStanding, 0), JPH::Quat::sIdentity(), new JPH::CapsuleShape(0.5f * characterHeightStanding, characterRadiusStanding)).Create().Get();

    JPH::Ref<JPH::CharacterVirtualSettings> settings = new JPH::CharacterVirtualSettings();
    settings->mMaxSlopeAngle = playerPhysicsSettings.maxSlopeAngle;
	settings->mMaxStrength = playerPhysicsSettings.maxStrength;
	settings->mShape = standingShape;
	settings->mBackFaceMode = playerPhysicsSettings.backFaceMode;
	settings->mCharacterPadding = playerPhysicsSettings.characterPadding;
	settings->mPenetrationRecoverySpeed = playerPhysicsSettings.penetrationRecoverySpeed;
	settings->mPredictiveContactDistance = playerPhysicsSettings.predictiveContactDistance;
	settings->mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -characterRadiusStanding); // Accept contacts that touch the lower sphere of the capsule
	settings->mEnhancedInternalEdgeRemoval = playerPhysicsSettings.enhancedInternalEdgeRemoval;
	settings->mInnerBodyShape = playerPhysicsSettings.createInnerBody ? standingShape : nullptr;
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
    if (!playerPhysicsSettings.enableStickToFloor)
        settings.mStickToFloorStepDown = JPH::Vec3::sZero();
    else
        settings.mStickToFloorStepDown = -character->GetUp() * settings.mStickToFloorStepDown.Length();

    if (!playerPhysicsSettings.enableWalkStairs)
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

    bool playerControlsHorizontalVelocity = canControlMovementDuringJump || character->IsSupported();
	if (playerControlsHorizontalVelocity) {
		// Smooth the player input
		desiredVelocity = enableCharacterInertia? 0.25f * moveDirection * movespeed + 0.75f * desiredVelocity : moveDirection * movespeed;

		// True if the player intended to move
		allowSliding = !moveDirection.IsNearZero();
	}
	else {
		// While in air we allow sliding
		allowSliding = true;
	}

	// Update the character rotation and its up vector to match the up vector set by the user settings
	JPH::Quat characterUpRotation = MathToJolt(camera.getRotation());
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
		&& (enableCharacterInertia?
			movingTowardsGround													// Inertia enabled: And not moving away from ground
			: !character->IsSlopeTooSteep(character->GetGroundNormal())))			// Inertia disabled: And not on a slope that is too steep
	{
		// Assume velocity of ground when on ground
		newVelocity = groundVelocity;

		// Jump
		if (jumping && movingTowardsGround)
			newVelocity += jumpSpeed * character->GetUp();
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