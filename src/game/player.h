#pragma once

#include "core/application_info.h"
#include "core/camera.h"
#include "input/input.h"

#include "physics/helpers.h"
#include "physics/physics.h"

struct PlayerPhysicsSettings
{
    float maxSlopeAngle = glm::radians(45.0f);
    float maxStrength = 100.0f;
    float characterPadding = 0.02f;
    float penetrationRecoverySpeed = 1.0f;
    float predictiveContactDistance = 0.1f;
    bool  enableWalkStairs = true;
    bool  enableStickToFloor = true;
    bool  enhancedInternalEdgeRemoval = false;
    bool  createInnerBody = false;
    bool  playerCanPushOtherCharacters = false;
    bool  otherCharactersCanPushPlayer = false;

    JPH::EBackFaceMode backFaceMode = JPH::EBackFaceMode::CollideWithBackFaces;
};


class Player
{
public:
    void initialize(Physics *physics, Input *input, const ApplicationInfo &appInfo);
    void update(float deltaTime);
    void processInput(float deltaTime);

    int32_t    getMeshId() const { return meshId; }
    const mat4 getTransform() const { return JoltToMath(worldTransform); }
    Camera    &getCamera() { return camera; }
    void       setKeyboardInput(bool mode) { canMove = mode; camera.setKeyboardInput(mode); }
    void       setMouseInput(bool mode) { camera.setMouseInput(mode); }

private:
    int32_t    meshId = -1;
    JPH::Mat44 worldTransform = JPH::Mat44::sIdentity();

    JPH::Vec3 moveDirection = JPH::Vec3::sZero();
    bool      jumping = false;
    bool      canMove = true;

    Camera   camera;
    Input   *input;
    Physics *physics;

    //
    // Physics
    //
    void initializePhysics();
    void updatePhysics(float deltaTime);
    void updatePhysicsController(float deltaTime);

    // Character size
    const float characterHeightStanding = 2.0f;
    const float characterRadiusStanding = 0.5f;

	PlayerPhysicsSettings playerPhysicsSettings = PlayerPhysicsSettings();

    JPH::Ref<JPH::CharacterVirtual> character;
    JPH::RefConst<JPH::Shape>       standingShape;
    JPH::Vec3                       desiredVelocity = JPH::Vec3::sZero();

    // Character movement settings
    bool  enableCharacterInertia = true;
    bool  allowSliding = false; // True when the player is pressing movement controls
    bool  canControlMovementDuringJump = true; ///< If false the character cannot change movement direction in mid air
    float movespeed = 6.0f;
    float jumpSpeed = 4.0f;
};