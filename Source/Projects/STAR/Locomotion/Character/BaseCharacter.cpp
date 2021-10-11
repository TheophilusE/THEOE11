
#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Math/EasingFunctions.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>

#include "BaseCharacter.h"

Character::Character(Context* context)
    : LogicComponent(context)
    , onGround_(false)
    , okToJump_(true)
    , inAirTimer_(0.0f)
    , isWalk(false)
    , isSprint(false)
{
    // Only the physics update event is needed: unsubscribe from the rest for optimization
    SetUpdateEventMask(USE_FIXEDUPDATE);
}

void Character::RegisterObject(Context* context)
{
    context->RegisterFactory<Character>();

    // These macros register the class attributes to the Context for automatic load / save handling.
    // We specify the Default attribute mode which means it will be used both for saving into file, and network
    // replication
    URHO3D_ATTRIBUTE("Controls Yaw", float, controls_.yaw_, 0.0f, AM_DEFAULT);
    URHO3D_ATTRIBUTE("Controls Pitch", float, controls_.pitch_, 0.0f, AM_DEFAULT);
    URHO3D_ATTRIBUTE("On Ground", bool, onGround_, false, AM_DEFAULT);
    URHO3D_ATTRIBUTE("OK To Jump", bool, okToJump_, true, AM_DEFAULT);
    URHO3D_ATTRIBUTE("In Air Timer", float, inAirTimer_, 0.0f, AM_DEFAULT);
}

void Character::Start()
{
    // Component has been inserted into its scene node. Subscribe to events now
    SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(Character, HandleNodeCollision));
}

void Character::FixedUpdate(float timeStep)
{
    /// \todo Could cache the components for faster access instead of finding them each frame
    auto* body = GetComponent<RigidBody>();
    auto* animCtrl = node_->GetComponent<AnimationController>(true);

    // Update the in air timer. Reset if grounded
    if (!onGround_)
    {
        inAirTimer_ += timeStep;
    }
    else
    {
        inAirTimer_ = 0.0f;
    }
    // When character has been in air less than 1/10 second, it's still interpreted as being on ground
    bool softGrounded = inAirTimer_ < INAIR_THRESHOLD_TIME;

    // Update movement & animation
    const Quaternion& rot = node_->GetRotation();
    Vector3 moveDir = Vector3::ZERO;
    const Vector3& velocity = body->GetLinearVelocity();
    // Velocity on the XZ plane
    Vector3 planeVelocity(velocity.x_, 0.0f, velocity.z_);

    if (controls_.IsDown(CTRL_FORWARD))
    {
        moveDir += Vector3::FORWARD;
    }
    if (controls_.IsDown(CTRL_BACK))
    {
        moveDir += Vector3::BACK;
    }
    if (controls_.IsDown(CTRL_LEFT))
    {
        moveDir += Vector3::LEFT;
    }
    if (controls_.IsDown(CTRL_RIGHT))
    {
        moveDir += Vector3::RIGHT;
    }

    if (controls_.IsPressed(CTRL_WALK_RUN, controlsPrev_))
    {
        isWalk = !isWalk;
    }

    if (controls_.IsDown(CTRL_SPRINT))
    {
        isSprint = true;
    }
    else
    {
        isSprint = false;
    }

    // Normalize move vector so that diagonal strafing is not faster
    if (moveDir.LengthSquared() > 0.0f)
        moveDir.Normalize();

    // If in air, allow control, but slower than when on ground
    if (!isSprint)
    {
        if (isWalk)
        {
            body->ApplyImpulse(rot * moveDir * (softGrounded ? WALK_FORCE : INAIR_MOVE_FORCE));
        }
        else
        {
            body->ApplyImpulse(rot * moveDir * (softGrounded ? RUN_FORCE : INAIR_MOVE_FORCE));
        }
    }
    else
    {
        body->ApplyImpulse(rot * moveDir * (softGrounded ? SPRINT_FORCE : INAIR_MOVE_FORCE));
    }

    if (softGrounded)
    {
        // When on ground, apply a braking force to limit maximum ground velocity
        Vector3 brakeForce = -planeVelocity * BRAKE_FORCE;
        body->ApplyImpulse(brakeForce);

        // Jump. Must release jump control between jumps
        if (controls_.IsDown(CTRL_JUMP))
        {
            if (okToJump_)
            {
                body->ApplyImpulse(Vector3::UP * JUMP_FORCE);
                okToJump_ = false;
                animCtrl->PlayExclusive(
                    "Models/Locomotion/Manneqin/Animations/Base/InAir/ALS_N_JumpLoop_Unreal Take.ani", 0, false, 0.2f);
            }
        }
        else
        {
            okToJump_ = true;
        }
    }

    if (!onGround_)
    {
        if (inAirTimer_ < 1.4f)
        {
            animCtrl->PlayExclusive("Models/Locomotion/Manneqin/Animations/Base/InAir/ALS_N_FallLoop_Unreal Take.ani",
                                    0, false, 0.2f);
        }
        else
        {
            animCtrl->PlayExclusive("Models/Locomotion/Manneqin/Animations/Base/InAir/ALS_Flail_Unreal Take.ani", 0,
                                    true, 0.2f);
        }
    }
    else
    {
        // Play walk animation if moving on ground, otherwise fade it out
        if (softGrounded && !moveDir.Equals(Vector3::ZERO) && !isSprint)
        {
            if (isWalk)
            {
                animCtrl->PlayExclusive(
                    "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Walk_F_Unreal Take.ani", 0, true,
                    0.2f);
                // Set animation speed proportional to velocity
                animCtrl->SetSpeed("Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Walk_F_Unreal Take.ani",
                                   planeVelocity.Length() * 0.5f);
            }
            else
            {
                animCtrl->PlayExclusive(
                    "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Run_F_Unreal Take.ani", 0, true, 0.2f);
                // Set animation speed proportional to velocity
                animCtrl->SetSpeed("Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Run_F_Unreal Take.ani",
                                   planeVelocity.Length() * 0.3f);
            }
        }
        else if (softGrounded && !moveDir.Equals(Vector3::ZERO) && isSprint)
        {
            animCtrl->PlayExclusive(
                "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Sprint_F_Impulse_Unreal Take.ani", 0, true,
                0.2f);
            // Set animation speed proportional to velocity
            animCtrl->SetSpeed(
                "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Sprint_F_Impulse_Unreal Take.ani",
                planeVelocity.Length() * 0.175f);
        }
        else
        {
            animCtrl->PlayExclusive("Models/Locomotion/Manneqin/Animations/Base/BasePoses/ALS_N_Pose_Unreal Take.ani",
                                    0, true, 0.2f);
        }
    }

    // Reset grounded flag for next frame
    onGround_ = false;
}

void Character::HandleNodeCollision(StringHash eventType, VariantMap& eventData)
{
    // Check collision contacts and see if character is standing on ground (look for a contact that has near vertical
    // normal)
    using namespace NodeCollision;

    MemoryBuffer contacts(eventData[P_CONTACTS].GetBuffer());

    while (!contacts.IsEof())
    {
        Vector3 contactPosition = contacts.ReadVector3();
        Vector3 contactNormal = contacts.ReadVector3();
        /*float contactDistance = */ contacts.ReadFloat();
        /*float contactImpulse = */ contacts.ReadFloat();

        // If contact is below node center and pointing up, assume it's a ground contact
        if (contactPosition.y_ < (node_->GetPosition().y_ + 1.0f))
        {
            float level = contactNormal.y_;
            if (level > 0.75)
                onGround_ = true;
        }
    }
}
