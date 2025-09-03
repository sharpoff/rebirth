#include <rebirth/physics/physics_helpers.h>
#include <rebirth/physics/physics_system.h>
#include <rebirth/util/logger.h>

namespace rebirth
{

void PhysicsSystem::initialize()
{
    // Register allocation hook.
    JPH::RegisterDefaultAllocator();

    JPH::Factory::sInstance = new JPH::Factory();

    // Register all physics types with the factory and install their collision handlers with the CollisionDispatch class.
    JPH::RegisterTypes();

    // We need a temp allocator for temporary allocations during the physics update. We're
    // pre-allocating 10 MB to avoid having to do allocations during the physics update.
    tempAllocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024);

    // We need a job system that will execute physics jobs on multiple threads.
    jobSystem = new JPH::JobSystemThreadPool(
        JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, JPH::thread::hardware_concurrency() - 1
    );

    physicsSystem.Init(
        maxBodies, numBodyMutexes, maxBodies, maxContactConstraints, broadPhaseLayerInterface,
        objectVsBroadPhaseLayerFilter, objectVsObjectFilter
    );

    physicsSystem.SetBodyActivationListener(&bodyActivationListener);
    physicsSystem.SetContactListener(&contactListener);
}

void PhysicsSystem::shutdown()
{
    JPH::BodyInterface &bodyInterface = physicsSystem.GetBodyInterface();

    for (auto &rigidBody : rigidBodies) {
        bodyInterface.RemoveBody(rigidBody.bodyId);
        bodyInterface.DestroyBody(rigidBody.bodyId);
    }

    JPH::UnregisterTypes();

    delete tempAllocator;
    delete jobSystem;
}

RigidBodyID PhysicsSystem::createBox(Transform transform, vec3 halfExtent, bool isStatic)
{
    JPH::BodyInterface &bodyInterface = physicsSystem.GetBodyInterface();

    JPH::BoxShapeSettings settings(toJolt(halfExtent));
    settings.SetEmbedded();

    RigidBody rigidBody;
    JPH::Shape::ShapeResult result = settings.Create();
    if (result.IsValid()) {
        rigidBody.shape = result.Get();
    } else {
        util::logError("Failed to create physics box: ", result.GetError());
        return -1;
    }

    vec3 position = transform.getPosition();
    quat rotation = transform.getRotation();

    rigidBody.bodyId = bodyInterface.CreateAndAddBody(
        JPH::BodyCreationSettings(
            rigidBody.shape.GetPtr(), toJolt(position), toJolt(rotation),
            isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic,
            isStatic ? Layers::NON_MOVING : Layers::MOVING
        ),
        isStatic ? JPH::EActivation::DontActivate : JPH::EActivation::Activate
    );

    bodyInterface.SetLinearVelocity(rigidBody.bodyId, JPH::Vec3(0.0f, -5.0f, 0.0f));

    rigidBodies.push_back(rigidBody);
    return rigidBodies.size() - 1;
}

RigidBodyID PhysicsSystem::createSphere(Transform transform, float radius, bool isStatic)
{
    JPH::BodyInterface &bodyInterface = physicsSystem.GetBodyInterface();

    JPH::SphereShapeSettings settings(radius);
    settings.SetEmbedded();

    RigidBody rigidBody;
    JPH::Shape::ShapeResult result = settings.Create();
    if (result.IsValid()) {
        rigidBody.shape = result.Get();
    } else {
        util::logError("Failed to create physics sphere: ", result.GetError());
        return -1;
    }

    vec3 position = transform.getPosition();
    quat rotation = transform.getRotation();

    rigidBody.bodyId = bodyInterface.CreateAndAddBody(
        JPH::BodyCreationSettings(
            rigidBody.shape.GetPtr(), toJolt(position), toJolt(rotation),
            isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic,
            isStatic ? Layers::NON_MOVING : Layers::MOVING
        ),
        isStatic ? JPH::EActivation::DontActivate : JPH::EActivation::Activate
    );

    if (!isStatic)
        bodyInterface.SetLinearVelocity(rigidBody.bodyId, JPH::Vec3(0.0f, -5.0f, 0.0f));

    rigidBodies.push_back(rigidBody);
    return rigidBodies.size() - 1;
}

RigidBodyID PhysicsSystem::createShape(
    JPH::Ref<JPH::Shape> &shape,
    JPH::BodyCreationSettings settings,
    JPH::EActivation activation
)
{
    JPH::BodyInterface &bodyInterface = physicsSystem.GetBodyInterface();

    RigidBody rigidBody;
    rigidBody.shape = shape;
    rigidBody.bodyId = bodyInterface.CreateAndAddBody(settings, activation);

    rigidBodies.push_back(rigidBody);
    return rigidBodies.size() - 1;
}

RigidBodyID PhysicsSystem::addShape(
    JPH::Ref<JPH::Shape> &shape,
    JPH::BodyID bodyId
)
{
    RigidBody rigidBody;
    rigidBody.shape = shape;
    rigidBody.bodyId = bodyId;

    rigidBodies.push_back(rigidBody);
    return rigidBodies.size() - 1;
}

void PhysicsSystem::removeRigidBody(RigidBodyID id)
{
    if (id > -1)
        rigidBodies.erase(rigidBodies.begin() + id);
}

void PhysicsSystem::setFriction(RigidBodyID id, float friction)
{
    JPH::BodyInterface &bodyInterface = physicsSystem.GetBodyInterface();
    bodyInterface.SetFriction(getRigidBody(id).bodyId, friction);
}

void PhysicsSystem::update(float dt)
{
    float physicsDeltaTime = 1.0f / 60.0f;

    physicsSystem.Update(physicsDeltaTime, 1, tempAllocator, jobSystem);
}

vec3 PhysicsSystem::getPosition(RigidBodyID id)
{
    RigidBody &rigidBody = rigidBodies[id];

    JPH::BodyInterface &bodyInterface = physicsSystem.GetBodyInterface();
    return toGLM(bodyInterface.GetPosition(rigidBody.bodyId));
}

quat PhysicsSystem::getRotation(RigidBodyID id)
{
    RigidBody &rigidBody = rigidBodies[id];

    JPH::BodyInterface &bodyInterface = physicsSystem.GetBodyInterface();
    return toGLM(bodyInterface.GetRotation(rigidBody.bodyId));
}

} // namespace rebirth