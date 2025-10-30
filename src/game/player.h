#pragma once

#include "core/application_info.h"
#include "core/camera.h"
#include "input/input.h"

#include "physics/physics.h"
#include "physics/helpers.h"

class Player
{
public:
    void initialize(Physics *physics, Input *input, const ApplicationInfo &appInfo);
    void update(float deltaTime);
    void processInput(float deltaTime);

    int32_t getMeshId() const { return meshId; }
    const mat4 getTransform() const { return JoltToMath(worldTransform); }
    Camera &getCamera() { return camera; }
    void setKeyboardInput(bool mode);
    void setMouseInput(bool mode);

private:
    int32_t meshId = -1;
    JPH::Mat44 worldTransform = JPH::Mat44::sIdentity();

    JPH::Vec3 moveDir = JPH::Vec3::sZero();
    bool jumping = false;
    bool canMove = true;

    //
    // Physics
    //
    void initializePhysics();
    void updatePhysics(float deltaTime);
    void updatePhysicsController(float deltaTime);

    JPH::Ref<JPH::CharacterVirtual> character;
    JPH::Vec3 desiredVelocity = JPH::Vec3::sZero();

    // Character movement settings
    bool sEnableCharacterInertia = true;
    bool allowSliding = false; // True when the player is pressing movement controls

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

    // Character movement properties
    bool  sControlMovementDuringJump = true; ///< If false the character cannot change movement direction in mid air
    float sCharacterSpeed = 6.0f;
    float sJumpSpeed = 4.0f;

    Camera camera;

    Input   *input;
    Physics *physics;
};