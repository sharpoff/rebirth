#include "game/player.h"

#include "physics/layers.h"
#include "physics/constants.h"
#include "Jolt/Geometry/Plane.h"
#include "physics/physics.h"

Player::Player(Physics *physics, Camera *camera)
{
    camera_ = camera;
    physics_ = physics;
}

void Player::update(float deltaTime)
{
}

JPH::Ref<JPH::CharacterVirtualSettings> Player::getCharacterSettings()
{
    JPH::Ref<JPH::CharacterVirtualSettings> settings = new JPH::CharacterVirtualSettings();
	settings->mMaxSlopeAngle = sMaxSlopeAngle;
	settings->mMaxStrength = sMaxStrength;
	settings->mShape = physics_->getShapeByName("character_standing");
	settings->mBackFaceMode = sBackFaceMode;
	settings->mCharacterPadding = sCharacterPadding;
	settings->mPenetrationRecoverySpeed = sPenetrationRecoverySpeed;
	settings->mPredictiveContactDistance = sPredictiveContactDistance;
	settings->mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -kCharacterRadiusStanding); // Accept contacts that touch the lower sphere of the capsule
	settings->mEnhancedInternalEdgeRemoval = sEnhancedInternalEdgeRemoval;
	settings->mInnerBodyShape = sCreateInnerBody ? physics_->getShapeByName("character_inner_standing") : nullptr;
	settings->mInnerBodyLayer = Layers::MOVING;
    return settings;
}