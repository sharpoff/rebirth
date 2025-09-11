#include <rebirth/game/entities/car.h>

#include <rebirth/physics/physics_helpers.h>
#include <rebirth/physics/physics_system.h>

#include <rebirth/graphics/renderer.h>
#include <rebirth/input/input.h>

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Vehicle/WheeledVehicleController.h>

Car::Car(vec3 position)
{
    entityType = EntityType::Car;

    JPH::PhysicsSystem &jphPhysicsSystem = g_physicsSystem.getPhysicsSystem();

    const float wheel_radius = 0.3f;
    const float wheel_width = 0.1f;
    const float half_vehicle_length = 2.0f;
    const float half_vehicle_width = 0.9f;
    const float half_vehicle_height = 0.2f;

    mCollisionTester = new JPH::VehicleCollisionTesterCastCylinder(Layers::MOVING);

    // create body
    JPH::Ref<JPH::Shape> car_shape = JPH::OffsetCenterOfMassShapeSettings(JPH::Vec3(0, -half_vehicle_height, 0), new JPH::BoxShape(JPH::Vec3(half_vehicle_width, half_vehicle_height, half_vehicle_length))).Create().Get();

    JPH::BodyCreationSettings car_body_settings(car_shape, MathToJolt(position), JPH::Quat::sRotation(JPH::Vec3::sAxisZ(), sInitialRollAngle), JPH::EMotionType::Dynamic, Layers::MOVING);
    car_body_settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
    car_body_settings.mMassPropertiesOverride.mMass = 1500.0f;
    car_body_settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
    car_body_settings.mMassPropertiesOverride.mMass = 1500.0f;
    rigidBodyId = g_physicsSystem.createShape(car_shape, car_body_settings, JPH::EActivation::Activate);

    // Create vehicle constraint
    JPH::VehicleConstraintSettings vehicle;
    vehicle.mDrawConstraintSize = 0.1f;
    vehicle.mMaxPitchRollAngle = sMaxRollAngle;

    // Suspension direction
    JPH::Vec3 front_suspension_dir = JPH::Vec3(JPH::Tan(sFrontSuspensionSidewaysAngle), -1, JPH::Tan(sFrontSuspensionForwardAngle)).Normalized();
    JPH::Vec3 front_steering_axis = JPH::Vec3(-JPH::Tan(sFrontKingPinAngle), 1, -JPH::Tan(sFrontCasterAngle)).Normalized();
    JPH::Vec3 front_wheel_up = JPH::Vec3(JPH::Sin(sFrontCamber), JPH::Cos(sFrontCamber), 0);
    JPH::Vec3 front_wheel_forward = JPH::Vec3(-JPH::Sin(sFrontToe), 0, JPH::Cos(sFrontToe));
    JPH::Vec3 rear_suspension_dir = JPH::Vec3(JPH::Tan(sRearSuspensionSidewaysAngle), -1, JPH::Tan(sRearSuspensionForwardAngle)).Normalized();
    JPH::Vec3 rear_steering_axis = JPH::Vec3(-JPH::Tan(sRearKingPinAngle), 1, -JPH::Tan(sRearCasterAngle)).Normalized();
    JPH::Vec3 rear_wheel_up = JPH::Vec3(JPH::Sin(sRearCamber), JPH::Cos(sRearCamber), 0);
    JPH::Vec3 rear_wheel_forward = JPH::Vec3(-JPH::Sin(sRearToe), 0, JPH::Cos(sRearToe));
    JPH::Vec3 flip_x(-1, 1, 1);

    // Wheels, left front
    JPH::WheelSettingsWV *w1 = new JPH::WheelSettingsWV;
    w1->mPosition = JPH::Vec3(half_vehicle_width, -0.9f * half_vehicle_height, half_vehicle_length - 2.0f * wheel_radius);
    w1->mSuspensionDirection = front_suspension_dir;
    w1->mSteeringAxis = front_steering_axis;
    w1->mWheelUp = front_wheel_up;
    w1->mWheelForward = front_wheel_forward;
    w1->mSuspensionMinLength = sFrontSuspensionMinLength;
    w1->mSuspensionMaxLength = sFrontSuspensionMaxLength;
    w1->mSuspensionSpring.mFrequency = sFrontSuspensionFrequency;
    w1->mSuspensionSpring.mDamping = sFrontSuspensionDamping;
    w1->mMaxSteerAngle = sMaxSteeringAngle;
    w1->mMaxHandBrakeTorque = 0.0f; // Front wheel doesn't have hand brake

    // Right front
    JPH::WheelSettingsWV *w2 = new JPH::WheelSettingsWV;
    w2->mPosition = JPH::Vec3(-half_vehicle_width, -0.9f * half_vehicle_height, half_vehicle_length - 2.0f * wheel_radius);
    w2->mSuspensionDirection = flip_x * front_suspension_dir;
    w2->mSteeringAxis = flip_x * front_steering_axis;
    w2->mWheelUp = flip_x * front_wheel_up;
    w2->mWheelForward = flip_x * front_wheel_forward;
    w2->mSuspensionMinLength = sFrontSuspensionMinLength;
    w2->mSuspensionMaxLength = sFrontSuspensionMaxLength;
    w2->mSuspensionSpring.mFrequency = sFrontSuspensionFrequency;
    w2->mSuspensionSpring.mDamping = sFrontSuspensionDamping;
    w2->mMaxSteerAngle = sMaxSteeringAngle;
    w2->mMaxHandBrakeTorque = 0.0f; // Front wheel doesn't have hand brake

    // Left rear
    JPH::WheelSettingsWV *w3 = new JPH::WheelSettingsWV;
    w3->mPosition = JPH::Vec3(half_vehicle_width, -0.9f * half_vehicle_height, -half_vehicle_length + 2.0f * wheel_radius);
    w3->mSuspensionDirection = rear_suspension_dir;
    w3->mSteeringAxis = rear_steering_axis;
    w3->mWheelUp = rear_wheel_up;
    w3->mWheelForward = rear_wheel_forward;
    w3->mSuspensionMinLength = sRearSuspensionMinLength;
    w3->mSuspensionMaxLength = sRearSuspensionMaxLength;
    w3->mSuspensionSpring.mFrequency = sRearSuspensionFrequency;
    w3->mSuspensionSpring.mDamping = sRearSuspensionDamping;
    w3->mMaxSteerAngle = 0.0f;

    // Right rear
    JPH::WheelSettingsWV *w4 = new JPH::WheelSettingsWV;
    w4->mPosition = JPH::Vec3(-half_vehicle_width, -0.9f * half_vehicle_height, -half_vehicle_length + 2.0f * wheel_radius);
    w4->mSuspensionDirection = flip_x * rear_suspension_dir;
    w4->mSteeringAxis = flip_x * rear_steering_axis;
    w4->mWheelUp = flip_x * rear_wheel_up;
    w4->mWheelForward = flip_x * rear_wheel_forward;
    w4->mSuspensionMinLength = sRearSuspensionMinLength;
    w4->mSuspensionMaxLength = sRearSuspensionMaxLength;
    w4->mSuspensionSpring.mFrequency = sRearSuspensionFrequency;
    w4->mSuspensionSpring.mDamping = sRearSuspensionDamping;
    w4->mMaxSteerAngle = 0.0f;

    vehicle.mWheels = {w1, w2, w3, w4};

    for (JPH::WheelSettings *w : vehicle.mWheels) {
        w->mRadius = wheel_radius;
        w->mWidth = wheel_width;
    }

    JPH::WheeledVehicleControllerSettings *controller = new JPH::WheeledVehicleControllerSettings;
    vehicle.mController = controller;

    // Differential
    controller->mDifferentials.resize(sFourWheelDrive ? 2 : 1);
    controller->mDifferentials[0].mLeftWheel = 0;
    controller->mDifferentials[0].mRightWheel = 1;
    if (sFourWheelDrive) {
        controller->mDifferentials[1].mLeftWheel = 2;
        controller->mDifferentials[1].mRightWheel = 3;

        // Split engine torque
        controller->mDifferentials[0].mEngineTorqueRatio =
            controller->mDifferentials[1].mEngineTorqueRatio = 0.5f;
    }

    // Anti rollbars
    if (sAntiRollbar) {
        vehicle.mAntiRollBars.resize(2);
        vehicle.mAntiRollBars[0].mLeftWheel = 0;
        vehicle.mAntiRollBars[0].mRightWheel = 1;
        vehicle.mAntiRollBars[1].mLeftWheel = 2;
        vehicle.mAntiRollBars[1].mRightWheel = 3;
    }

    RigidBody &rigidBody = g_physicsSystem.getRigidBody(rigidBodyId);
    mVehicleConstraint = new JPH::VehicleConstraint(*rigidBody.getBody(), vehicle);

    // The vehicle settings were tweaked with a buggy implementation of the longitudinal tire impulses, this meant that PhysicsSettings::mNumVelocitySteps times more impulse
    // could be applied than intended. To keep the behavior of the vehicle the same we increase the max longitudinal impulse by the same factor. In a future version the vehicle
    // will be retweaked.
    static_cast<JPH::WheeledVehicleController *>(mVehicleConstraint->GetController())
        ->SetTireMaxImpulseCallback([](uint, float &outLongitudinalImpulse, float &outLateralImpulse, float inSuspensionImpulse, float inLongitudinalFriction, float inLateralFriction, float, float, float) {
            outLongitudinalImpulse = 10.0f * inLongitudinalFriction * inSuspensionImpulse;
            outLateralImpulse = inLateralFriction * inSuspensionImpulse;
        });

    jphPhysicsSystem.AddConstraint(mVehicleConstraint);
    jphPhysicsSystem.AddStepListener(mVehicleConstraint);

    // // should be updated
    // update(0.001f);

    // Set the collision tester
    mVehicleConstraint->SetVehicleCollisionTester(mCollisionTester);
}

void Car::update(float deltaTime)
{
    if (mRight != 0.0f || mForward != 0.0f || mBrake != 0.0f || mHandBrake != 0.0f)
        g_physicsSystem.activateBody(rigidBodyId);

    JPH::WheeledVehicleController *controller = static_cast<JPH::WheeledVehicleController *>(mVehicleConstraint->GetController());

    // Update vehicle statistics
    controller->GetEngine().mMaxTorque = sMaxEngineTorque;
    controller->GetTransmission().mClutchStrength = sClutchStrength;

    // Set slip ratios to the same for everything
    float limited_slip_ratio = sLimitedSlipDifferentials ? 1.4f : FLT_MAX;
    controller->SetDifferentialLimitedSlipRatio(limited_slip_ratio);
    for (JPH::VehicleDifferentialSettings &d : controller->GetDifferentials())
        d.mLimitedSlipRatio = limited_slip_ratio;

    // Pass the input on to the constraint
    controller->SetDriverInput(mForward, mRight, mBrake, mHandBrake);
}

void Car::draw(Renderer &renderer)
{
    RigidBody &rigidBody = g_physicsSystem.getRigidBody(getRigidBodyId());

    // draw wheels
    for (int i = 0; i < 4; i++) {
        renderer.drawModel(renderer.getCylinderPrimitive(), getWheelTransform(i));
    }

    // draw body
    Transform transform(rigidBody.getPosition(), rigidBody.getRotation(), vec3(2.0f, 0.05f, 2.6f));
    renderer.drawModel(renderer.getCubePrimitive(), transform);
}

void Car::processInput()
{
    Input &input = g_input;

    mForward = 0.0f;
    if (input.isKeyPressed(KeyboardKey::W))
        mForward = 1.0f;
    else if (input.isKeyPressed(KeyboardKey::S))
        mForward = -1.0f;

    mBrake = 0.0f;
    if (mPreviousForward * mForward < 0.0f) {
        JPH::Body *body = g_physicsSystem.getRigidBody(getRigidBodyId()).getBody();

        // Get vehicle velocity in local space to the body of the vehicle
        float velocity = (body->GetRotation().Conjugated() * body->GetLinearVelocity()).GetZ();
        if ((mForward > 0.0f && velocity < -0.1f) || (mForward < 0.0f && velocity > 0.1f)) {
            // Brake while we've not stopped yet
            mForward = 0.0f;
            mBrake = 1.0f;
        } else {
            // When we've come to a stop, accept the new direction
            mPreviousForward = mForward;
        }
    }

    // Steering
    mRight = 0.0f;
    if (input.isKeyPressed(KeyboardKey::A))
        mRight = -1.0f;
    else if (input.isKeyPressed(KeyboardKey::D))
        mRight = 1.0f;
}

Transform Car::getWheelTransform(int wheel)
{
    JPH::RMat44 wheel_transform = mVehicleConstraint->GetWheelWorldTransform(wheel, JPH::Vec3::sAxisY(), JPH::Vec3::sAxisX());

    Transform transform = Transform(JoltToMath(wheel_transform));
    auto width = getWheelWidth(wheel);
    transform.scale(vec3(width, width, width));
    return transform;
}

float Car::getWheelRadius(int wheel)
{
    const JPH::WheelSettings *settings = mVehicleConstraint->GetWheels()[wheel]->GetSettings();
    return settings->mWidth;
}

float Car::getWheelWidth(int wheel)
{
    const JPH::WheelSettings *settings = mVehicleConstraint->GetWheels()[wheel]->GetSettings();
    return settings->mRadius;
}