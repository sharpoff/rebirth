#include "game/player.h"

#include "Jolt/Math/MathTypes.h"
#include "Jolt/Math/Vec3.h"
#include "core/camera.h"
#include "core/resource_manager.h"
#include "math/math.h"
#include "physics/helpers.h"
#include "util/util.h"

#include "physics/constants.h"

void Player::initialize(Physics *physics, Input *input, const ApplicationInfo &appInfo)
{
    assert(physics && input);

    this->physics = physics;
    this->input = input;

    // create camera
    camera.initialize(input);
    camera.setKeyboardInput(false); // turn off input checking, only direct position setting
    camera.setCameraType(CameraType::Orbit);
    camera.setPerspectiveInf(glm::radians(60.0f), float(appInfo.width) / appInfo.height, 0.1f);
    camera.setEyeUpOffset(kCharacterHeightStanding);
    camera.setEyeFrontOffset(kCharacterHeightStanding + 2.0f);
    camera.setTargetUpOffset(2.0f);

    initializePhysics();

    // Create character mesh
    // TODO: replace cylinder with a capsule
    {
        int                     segments = 16;
        eastl::vector<Vertex>   vertices = util::generateCylinderVertices(kCharacterRadiusStanding, kCharacterHeightStanding, segments);
        eastl::vector<uint32_t> indices = util::generateCylinderIndices(segments);

        uint32_t vertexOffset = ResourceManager::get()->addVertices(vertices);
        uint32_t indexOffset = ResourceManager::get()->addIndices(indices);

        Mesh &cylinder = ResourceManager::get()->createNewMesh("CharacterMesh");
        cylinder.primitives.push_back(Primitive{
            .materialIndex = ResourceManager::get()->getMaterialIndexByName("checkerboard"),
            .indexOffset = indexOffset,
            .indexCount = uint32_t(indices.size()),
            .vertexOffset = vertexOffset,
            .vertexCount = uint32_t(vertices.size()),
        });

        meshId = ResourceManager::get()->getMeshIndex(&cylinder);
    }
}

void Player::update(float deltaTime)
{
    updatePhysics(deltaTime);

    // set camera position behind player
    camera.setPosition(math::getPosition(JoltToMath(worldTransform)));
    camera.update(deltaTime);
}

void Player::processInput(float deltaTime)
{
    if (!canMove) return;

    assert(input);

    moveDir = JPH::Vec3::sZero();
    if (input->getKey(KeyboardKey::W, InputAction::Pressed))
        moveDir.SetZ(-1);
    if (input->getKey(KeyboardKey::S, InputAction::Pressed))
        moveDir.SetZ(1);
    if (input->getKey(KeyboardKey::A, InputAction::Pressed))
        moveDir.SetX(-1);
    if (input->getKey(KeyboardKey::D, InputAction::Pressed))
        moveDir.SetX(1);
    moveDir = moveDir.NormalizedOr(JPH::Vec3::sZero());

    jumping = false;
    if (input->getKey(KeyboardKey::SPACE, InputAction::Pressed))
        jumping = true;
}

void Player::setKeyboardInput(bool mode)
{
    canMove = mode;
    camera.setKeyboardInput(mode);
}

void Player::setMouseInput(bool mode)
{
    canMove = mode;
    camera.setMouseInput(mode);
}