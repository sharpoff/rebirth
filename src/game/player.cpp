#include "game/player.h"

#include "core/camera.h"
#include "math/math.h"
#include "physics/helpers.h"

Player::Player(const PlayerCreateParams &params)
{
    assert(params.pInput && params.pPhysics);

    m_pPhysics = params.pPhysics;
    m_pInput = params.pInput;

    // create camera
    m_pCamera = new Camera({m_pInput});
    m_pCamera->setKeyboardInput(false); // turn off input checking, only direct position setting
    m_pCamera->setCameraType(CameraType::Orbit);
    m_pCamera->setPerspectiveInf(glm::radians(60.0f), float(params.appWidth) / params.appHeight, 0.1f);
    m_pCamera->setEyeUpOffset(characterHeightStanding);
    m_pCamera->setEyeFrontOffset(characterHeightStanding + 2.0f);
    m_pCamera->setTargetUpOffset(2.0f);

    initializePhysics();

    // // Create character mesh
    // // TODO: replace cylinder with a capsule
    // {
    //     int                     segments = 16;
    //     eastl::vector<Vertex>   vertices = util::generateCylinderVertices(characterRadiusStanding, characterHeightStanding, segments);
    //     eastl::vector<uint32_t> indices = util::generateCylinderIndices(segments);

    //     uint32_t vertexOffset = ResourceManager::get()->addVertices(vertices);
    //     uint32_t indexOffset = ResourceManager::get()->addIndices(indices);

    //     Mesh cylinder = {};
    //     cylinder.primitives.push_back(Primitive{
    //         .materialIndex = ResourceManager::get()->getMaterialIndexByName("checkerboard"),
    //         .indexOffset = indexOffset,
    //         .indexCount = uint32_t(indices.size()),
    //         .vertexOffset = vertexOffset,
    //         .vertexCount = uint32_t(vertices.size()),
    //     });
    //     meshId = ResourceManager::get()->addMesh(cylinder);
    // }
}

void Player::update(float deltaTime)
{
    updatePhysics(deltaTime);

    // set camera position behind player
    m_pCamera->setPosition(math::getPosition(JoltToMath(worldTransform)));
    m_pCamera->update(deltaTime);
}

void Player::processInput(float deltaTime)
{
    if (!canMove)
        return;

    assert(m_pInput);

    moveDirection = JPH::Vec3::sZero();
    if (m_pInput->getKey(KeyboardKey::W, InputAction::Pressed))
        moveDirection.SetZ(-1);
    if (m_pInput->getKey(KeyboardKey::S, InputAction::Pressed))
        moveDirection.SetZ(1);
    if (m_pInput->getKey(KeyboardKey::A, InputAction::Pressed))
        moveDirection.SetX(-1);
    if (m_pInput->getKey(KeyboardKey::D, InputAction::Pressed))
        moveDirection.SetX(1);
    moveDirection = moveDirection.NormalizedOr(JPH::Vec3::sZero());

    jumping = false;
    if (m_pInput->getKey(KeyboardKey::SPACE, InputAction::Pressed))
        jumping = true;
}

void Player::setKeyboardInput(bool mode)
{
    canMove = mode;
    m_pCamera->setKeyboardInput(mode);
}

void Player::setMouseInput(bool mode)
{
    m_pCamera->setMouseInput(mode);
}