#pragma once

#include "core/camera.h"
#include "game/entity.h"
#include "input/input.h"

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

struct PlayerCreateParams
{
    eastl::string m_name = "";
    Bounds        m_bounds = {};
    mat4          m_transform = mat4(1.0f);
    uint32_t      m_meshId = 0;
    uint32_t      m_customMaterialId = 0;

    uint32_t appWidth = 0;
    uint32_t appHeight = 0;
    Physics *pPhysics = nullptr;
    Input   *pInput = nullptr;
};

class Player : public Entity
{
public:
    Player(const PlayerCreateParams &params);
    void update(float deltaTime) override;
    void processInput(float deltaTime) override;

    Camera *getCamera() { return m_pCamera; }

    void setKeyboardInput(bool mode);
    void setMouseInput(bool mode);

private:
    JPH::Mat44 worldTransform = JPH::Mat44::sIdentity();

    JPH::Vec3 moveDirection = JPH::Vec3::sZero();
    bool      jumping = false;
    bool      canMove = true;

    // Character dimensions
    const float characterHeightStanding = 2.0f;
    const float characterRadiusStanding = 0.5f;

    Camera  *m_pCamera = nullptr;
    Input   *m_pInput = nullptr;
    Physics *m_pPhysics = nullptr;

    //
    // Physics
    //
    PlayerPhysicsSettings m_playerPhysicsSettings = PlayerPhysicsSettings();

    JPH::Ref<JPH::CharacterVirtual> m_character;
    JPH::RefConst<JPH::Shape>       m_standingShape;
    JPH::Vec3                       m_desiredVelocity = JPH::Vec3::sZero();

    // Character movement settings
    bool  enableCharacterInertia = true;
    bool  allowSliding = false; // True when the player is pressing movement controls
    bool  canControlMovementDuringJump = true; ///< If false the character cannot change movement direction in mid air
    float movespeed = 6.0f;
    float jumpSpeed = 4.0f;

    void initializePhysics();
    void updatePhysics(float deltaTime);
    void updatePhysicsController(float deltaTime);
};