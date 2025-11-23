#include "game/player.h"

#include "Jolt/Geometry/Plane.h"
#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"
#include "Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h"

#include "physics/helpers.h"
#include "physics/layers.h"

void Player::initializePhysics()
{
    m_standingShape = JPH::RotatedTranslatedShapeSettings(JPH::Vec3(0, 0.5f * characterHeightStanding + characterRadiusStanding, 0), JPH::Quat::sIdentity(), new JPH::CapsuleShape(0.5f * characterHeightStanding, characterRadiusStanding)).Create().Get();

    JPH::Ref<JPH::CharacterVirtualSettings> settings = new JPH::CharacterVirtualSettings();
    settings->mMaxSlopeAngle = m_playerPhysicsSettings.maxSlopeAngle;
	settings->mMaxStrength = m_playerPhysicsSettings.maxStrength;
	settings->mShape = m_standingShape;
	settings->mBackFaceMode = m_playerPhysicsSettings.backFaceMode;
	settings->mCharacterPadding = m_playerPhysicsSettings.characterPadding;
	settings->mPenetrationRecoverySpeed = m_playerPhysicsSettings.penetrationRecoverySpeed;
	settings->mPredictiveContactDistance = m_playerPhysicsSettings.predictiveContactDistance;
	settings->mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -characterRadiusStanding); // Accept contacts that touch the lower sphere of the capsule
	settings->mEnhancedInternalEdgeRemoval = m_playerPhysicsSettings.enhancedInternalEdgeRemoval;
	settings->mInnerBodyShape = m_playerPhysicsSettings.createInnerBody ? m_standingShape : nullptr;
	settings->mInnerBodyLayer = Layers::MOVING;

    m_character = m_pPhysics->createPlayer(settings, JPH::Vec3(0.0f, 10.0f, 3.0f));
	m_character->SetLinearVelocity(m_pPhysics->getSystem()->GetGravity());
}

void Player::updatePhysics(float deltaTime)
{
    if (!m_character) return;

	if (canMove)
		updatePhysicsController(deltaTime);

    // Settings for our update function
    JPH::CharacterVirtual::ExtendedUpdateSettings settings{};
    if (!m_playerPhysicsSettings.enableStickToFloor)
        settings.mStickToFloorStepDown = JPH::Vec3::sZero();
    else
        settings.mStickToFloorStepDown = -m_character->GetUp() * settings.mStickToFloorStepDown.Length();

    if (!m_playerPhysicsSettings.enableWalkStairs)
        settings.mWalkStairsStepUp = JPH::Vec3::sZero();
    else
        settings.mWalkStairsStepUp = m_character->GetUp() * settings.mWalkStairsStepUp.Length();

    JPH::PhysicsSystem *physicsSystem = m_pPhysics->getSystem();

    // Update the character position
    m_character->ExtendedUpdate(deltaTime,
        -m_character->GetUp() * physicsSystem->GetGravity().Length(),
        settings,
        physicsSystem->GetDefaultBroadPhaseLayerFilter(Layers::MOVING),
        physicsSystem->GetDefaultLayerFilter(Layers::MOVING),
        {},
        {},
        *m_pPhysics->getTempAllocator());

    worldTransform = m_character->GetWorldTransform();
}

void Player::updatePhysicsController(float deltaTime)
{
    JPH::PhysicsSystem *physicsSystem = m_pPhysics->getSystem();

    bool playerControlsHorizontalVelocity = canControlMovementDuringJump || m_character->IsSupported();
	if (playerControlsHorizontalVelocity) {
		// Smooth the player input
		m_desiredVelocity = enableCharacterInertia? 0.25f * moveDirection * movespeed + 0.75f * m_desiredVelocity : moveDirection * movespeed;

		// True if the player intended to move
		allowSliding = !moveDirection.IsNearZero();
	}
	else {
		// While in air we allow sliding
		allowSliding = true;
	}

	// Update the character rotation and its up vector to match the up vector set by the user settings
	JPH::Quat characterUpRotation = MathToJolt(m_pCamera->getRotation());
	m_character->SetUp(characterUpRotation.RotateAxisY());
	m_character->SetRotation(characterUpRotation);

	// A cheaper way to update the character's ground velocity,
	// the platforms that the character is standing on may have changed velocity
	m_character->UpdateGroundVelocity();

	// Determine new basic velocity
	JPH::Vec3 currentVerticalVelocity = m_character->GetLinearVelocity().Dot(m_character->GetUp()) * m_character->GetUp();
	JPH::Vec3 groundVelocity = m_character->GetGroundVelocity();
	JPH::Vec3 newVelocity;
	bool movingTowardsGround = (currentVerticalVelocity.GetY() - groundVelocity.GetY()) < 0.1f;
	if (m_character->GetGroundState() == JPH::CharacterVirtual::EGroundState::OnGround	// If on ground
		&& (enableCharacterInertia?
			movingTowardsGround													// Inertia enabled: And not moving away from ground
			: !m_character->IsSlopeTooSteep(m_character->GetGroundNormal())))			// Inertia disabled: And not on a slope that is too steep
	{
		// Assume velocity of ground when on ground
		newVelocity = groundVelocity;

		// Jump
		if (jumping && movingTowardsGround)
			newVelocity += jumpSpeed * m_character->GetUp();
	}
	else
		newVelocity = currentVerticalVelocity;

	// Gravity
	newVelocity += (characterUpRotation * physicsSystem->GetGravity()) * deltaTime;

	if (playerControlsHorizontalVelocity) {
		// Player input
		newVelocity += characterUpRotation * m_desiredVelocity;
	}
	else {
		// Preserve horizontal velocity
		JPH::Vec3 currentHorizontalVelocity = m_character->GetLinearVelocity() - currentVerticalVelocity;
		newVelocity += currentHorizontalVelocity;
	}

	// Update character velocity
	m_character->SetLinearVelocity(newVelocity);
}