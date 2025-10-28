#pragma once

#include "core/camera.h"
#include "entity.h"

#include "Jolt/Physics/Character/CharacterVirtual.h"
#include "Jolt/Physics/Collision/BackFaceMode.h"

class Player
{
public:
    Player(Physics *physics, Camera *camera);

    void update(float deltaTime);

private:
    JPH::Ref<JPH::CharacterVirtualSettings> getCharacterSettings();

    int32_t overrideMaterialId = -1;
    int32_t meshId = -1;

    //
    // Physics
    //

    vec3 velocity = vec3(0);

    bool allowSliding = false;

    JPH::Ref<JPH::CharacterVirtual> character_;

    // Character movement settings
    bool sEnableCharacterInertia = true;

    // configuration settings
    float sUpRotationX = 0;
    float sUpRotationZ = 0;
    float sMaxSlopeAngle = glm::radians(45.0f);
    float sMaxStrength = 100.0f;
    float sCharacterPadding = 0.02f;
    float sPenetrationRecoverySpeed = 1.0f;
    float sPredictiveContactDistance = 0.1f;
    bool  sEnableWalkStairs = true;
    bool  sEnableStickToFloor = true;
    bool  sEnhancedInternalEdgeRemoval = false;
    bool  sCreateInnerBody = false;
    bool  sPlayerCanPushOtherCharacters = true;
    bool  sOtherCharactersCanPushPlayer = true;

    JPH::EBackFaceMode sBackFaceMode = JPH::EBackFaceMode::CollideWithBackFaces;

    // True when the player is pressing movement controls
    bool mAllowSliding = false;

    // Track active contacts for debugging purposes
    JPH::Array<JPH::CharacterVirtual::ContactKey> activeContacts_;

    // Character movement properties
    bool  sControlMovementDuringJump = true; ///< If false the character cannot change movement direction in mid air
    float sCharacterSpeed = 6.0f;
    float sJumpSpeed = 4.0f;

    Camera  *camera_;
    Physics *physics_;
};