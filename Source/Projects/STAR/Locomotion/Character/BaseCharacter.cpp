
#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Math/EasingFunctions.h>
#include <Urho3D/Math/Ray.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/IK/IKEffector.h>
#include <Urho3D/IK/IKSolver.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Input/InputManager.h>

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
    URHO3D_ATTRIBUTE("On Ground", bool, onGround_, false, AM_DEFAULT);
    URHO3D_ATTRIBUTE("OK To Jump", bool, okToJump_, true, AM_DEFAULT);
    URHO3D_ATTRIBUTE("In Air Timer", float, inAirTimer_, 0.0f, AM_DEFAULT);
}

void Character::Start()
{
    // Component has been inserted into its scene node. Subscribe to events now
    SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(Character, HandleNodeCollision));
    SubscribeToEvent(GetNode(), E_SCENEDRAWABLEUPDATEFINISHED, URHO3D_HANDLER(Character, HandleSceneDrawableUpdateFinished));

    // Set Character Node
    characterNode_ = GetNode();

    // We need to attach two inverse kinematic effectors to Jack's feet to
    // control the grounding.
    leftFoot_ = characterNode_->GetChild("foot_l", true);
    rightFoot_ = characterNode_->GetChild("foot_r", true);
    leftEffector_ = leftFoot_->CreateComponent<IKEffector>();
    rightEffector_ = rightFoot_->CreateComponent<IKEffector>();
    // Control 2 segments up to the hips
    leftEffector_->SetChainLength(2);
    rightEffector_->SetChainLength(2);

    // For the effectors to work, an IKSolver needs to be attached to one of
    // the parent nodes. Typically, you want to place the solver as close as
    // possible to the effectors for optimal performance. Since in this case
    // we're solving the legs only, we can place the solver at the spine.
    Node* spine = characterNode_->GetChild("pelvis", true);
    URHO3D_LOGINFO(fmt::format("{}", true));
    solver_ = spine->CreateComponent<IKSolver>();

    URHO3D_LOGINFO(fmt::format("{}", true));

    // Two-bone solver is more efficient and more stable than FABRIK (but only
    // works for two bones, obviously).
    solver_->SetAlgorithm(IKSolver::TWO_BONE);

    // Disable auto-solving, which means we need to call Solve() manually
    solver_->SetFeature(IKSolver::AUTO_SOLVE, false);

    // Only enable this so the debug draw shows us the pose before solving.
    // This should NOT be enabled for any other reason (it does nothing and is
    // a waste of performance).
    solver_->SetFeature(IKSolver::UPDATE_ORIGINAL_POSE, true);
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

    if (InputManager::IsDown("MoveForward"))
    {
        moveDir += Vector3::FORWARD;
    }
    if (InputManager::IsDown("MoveBackward"))
    {
        moveDir += Vector3::BACK;
    }
    if (InputManager::IsDown("MoveLeft"))
    {
        moveDir += Vector3::LEFT;
    }
    if (InputManager::IsDown("MoveRight"))
    {
        moveDir += Vector3::RIGHT;
    }

    if (InputManager::IsPressed("ToggleWalk"))
    {
        isWalk = !isWalk;
    }

    if (InputManager::IsDown("ToggleSprint"))
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
        if (isWalk || (moveDir.x_ != 0))
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
        if (InputManager::IsPressed("Jump"))
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
                // Forward and Backwards
                if (moveDir.x_ == 0 && moveDir.z_ != 0)
                {
                    // Forward
                    if (moveDir.z_ > 0)
                    {
                        animCtrl->PlayExclusive(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Walk_F_Unreal Take.ani", 0,
                            true, 0.2f);
                        // Set animation speed proportional to velocity
                        animCtrl->SetSpeed(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Walk_F_Unreal Take.ani",
                            planeVelocity.Length() * 0.5f);
                    }
                    // Backward
                    if (moveDir.z_ < 0)
                    {
                        animCtrl->PlayExclusive(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Walk_B_Unreal Take.ani", 0,
                            true, 0.2f);
                        // Set animation speed proportional to velocity
                        animCtrl->SetSpeed(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Walk_B_Unreal Take.ani",
                            planeVelocity.Length() * 0.5f);
                    }
                }
                /// Left and Right
                if (moveDir.x_ != 0)
                {
                    // Right
                    if (moveDir.x_ > 0 && moveDir.z_ == 0)
                    {
                        animCtrl->PlayExclusive(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Walk_RF_Unreal Take.ani", 0,
                            true, 0.2f);
                        // Set animation speed proportional to velocity
                        animCtrl->SetSpeed(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Walk_RF_Unreal Take.ani",
                            planeVelocity.Length() * 0.5f);
                    }
                    // Left
                    if (moveDir.x_ < 0 && moveDir.z_ == 0)
                    {
                        animCtrl->PlayExclusive(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Walk_LF_Unreal Take.ani", 0,
                            true, 0.2f);
                        // Set animation speed proportional to velocity
                        animCtrl->SetSpeed(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Walk_LF_Unreal Take.ani",
                            planeVelocity.Length() * 0.5f);
                    }
                    // Right Front
                    if (moveDir.x_ > 0 && moveDir.z_ > 0)
                    {
                        animCtrl->PlayExclusive(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Walk_RF_Unreal Take.ani", 0,
                            true, 0.2f);
                        // Set animation speed proportional to velocity
                        animCtrl->SetSpeed(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Walk_RF_Unreal Take.ani",
                            planeVelocity.Length() * 0.5f);
                    }
                    // Left Front
                    if (moveDir.x_ < 0 && moveDir.z_ > 0)
                    {
                        animCtrl->PlayExclusive(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Walk_LF_Unreal Take.ani", 0,
                            true, 0.2f);
                        // Set animation speed proportional to velocity
                        animCtrl->SetSpeed(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Walk_LF_Unreal Take.ani",
                            planeVelocity.Length() * 0.5f);
                    }
                    // Right Back
                    if (moveDir.x_ > 0 && moveDir.z_ < 0)
                    {
                        animCtrl->PlayExclusive(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Walk_RB_Unreal Take.ani", 0,
                            true, 0.2f);
                        // Set animation speed proportional to velocity
                        animCtrl->SetSpeed(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Walk_RB_Unreal Take.ani",
                            planeVelocity.Length() * 0.5f);
                    }
                    // Left Back
                    if (moveDir.x_ < 0 && moveDir.z_ < 0)
                    {
                        animCtrl->PlayExclusive(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Walk_LB_Unreal Take.ani", 0,
                            true, 0.2f);
                        // Set animation speed proportional to velocity
                        animCtrl->SetSpeed(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Walk_LB_Unreal Take.ani",
                            planeVelocity.Length() * 0.5f);
                    }
                }
            }
            else
            {
                // Forward and Backwards
                if (moveDir.x_ == 0 && moveDir.z_ != 0)
                {
                    // Forward
                    if (moveDir.z_ > 0)
                    {
                        animCtrl->PlayExclusive(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Run_F_Unreal Take.ani", 0,
                            true, 0.2f);
                        // Set animation speed proportional to velocity
                        animCtrl->SetSpeed(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Run_F_Unreal Take.ani",
                            planeVelocity.Length() * 0.3f);
                    }
                    // Backward
                    if (moveDir.z_ < 0)
                    {
                        animCtrl->PlayExclusive(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Run_B_Unreal Take.ani", 0,
                            true, 0.2f);
                        // Set animation speed proportional to velocity
                        animCtrl->SetSpeed(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Run_B_Unreal Take.ani",
                            planeVelocity.Length() * 0.3f);
                    }
                }
                /// Left and Right
                if (moveDir.x_ != 0)
                {
                    // Right
                    if (moveDir.x_ > 0 && moveDir.z_ == 0)
                    {
                        animCtrl->PlayExclusive(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Walk_RF_Unreal Take.ani", 0,
                            true, 0.2f);
                        // Set animation speed proportional to velocity
                        animCtrl->SetSpeed(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Walk_RF_Unreal Take.ani",
                            planeVelocity.Length() * 0.5f);
                    }
                    // Left
                    if (moveDir.x_ < 0 && moveDir.z_ == 0)
                    {
                        animCtrl->PlayExclusive(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Walk_LF_Unreal Take.ani", 0,
                            true, 0.2f);
                        // Set animation speed proportional to velocity
                        animCtrl->SetSpeed(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Walk_LF_Unreal Take.ani",
                            planeVelocity.Length() * 0.5f);
                    }
                    // Right Front
                    if (moveDir.x_ > 0 && moveDir.z_ > 0)
                    {
                        animCtrl->PlayExclusive(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Walk_RF_Unreal Take.ani", 0,
                            true, 0.2f);
                        // Set animation speed proportional to velocity
                        animCtrl->SetSpeed(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Walk_RF_Unreal Take.ani",
                            planeVelocity.Length() * 0.5f);
                    }
                    // Left Front
                    if (moveDir.x_ < 0 && moveDir.z_ > 0)
                    {
                        animCtrl->PlayExclusive(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Walk_LF_Unreal Take.ani", 0,
                            true, 0.2f);
                        // Set animation speed proportional to velocity
                        animCtrl->SetSpeed(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Walk_LF_Unreal Take.ani",
                            planeVelocity.Length() * 0.5f);
                    }
                    // Right Back
                    if (moveDir.x_ > 0 && moveDir.z_ < 0)
                    {
                        animCtrl->PlayExclusive(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Walk_RB_Unreal Take.ani", 0,
                            true, 0.2f);
                        // Set animation speed proportional to velocity
                        animCtrl->SetSpeed(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Walk_RB_Unreal Take.ani",
                            planeVelocity.Length() * 0.5f);
                    }
                    // Left Back
                    if (moveDir.x_ < 0 && moveDir.z_ < 0)
                    {
                        animCtrl->PlayExclusive(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Walk_LB_Unreal Take.ani", 0,
                            true, 0.2f);
                        // Set animation speed proportional to velocity
                        animCtrl->SetSpeed(
                            "Models/Locomotion/Manneqin/Animations/Base/Locomotion/ALS_N_Walk_LB_Unreal Take.ani",
                            planeVelocity.Length() * 0.5f);
                    }
                }
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

void Character::DrawDebug()
{
    solver_->DrawDebugGeometry(true);
}

void Character::Setup()
{
    // Set Character Node
    characterNode_ = GetNode();

    // We need to attach two inverse kinematic effectors to Jack's feet to
    // control the grounding.
    leftFoot_ = characterNode_->GetChild("foot_l", true);
    rightFoot_ = characterNode_->GetChild("foot_r", true);
    leftEffector_ = leftFoot_->CreateComponent<IKEffector>();
    rightEffector_ = rightFoot_->CreateComponent<IKEffector>();
    // Control 2 segments up to the hips
    leftEffector_->SetChainLength(2);
    rightEffector_->SetChainLength(2);

    // For the effectors to work, an IKSolver needs to be attached to one of
    // the parent nodes. Typically, you want to place the solver as close as
    // possible to the effectors for optimal performance. Since in this case
    // we're solving the legs only, we can place the solver at the spine.
    Node* spine = characterNode_->GetChild("pelvis", true);
    URHO3D_LOGINFO(fmt::format("{}", true));
    solver_ = spine->CreateComponent<IKSolver>();

    URHO3D_LOGINFO(fmt::format("{}", true));

    // Two-bone solver is more efficient and more stable than FABRIK (but only
    // works for two bones, obviously).
    solver_->SetAlgorithm(IKSolver::TWO_BONE);

    // Disable auto-solving, which means we need to call Solve() manually
    solver_->SetFeature(IKSolver::AUTO_SOLVE, false);

    // Only enable this so the debug draw shows us the pose before solving.
    // This should NOT be enabled for any other reason (it does nothing and is
    // a waste of performance).
    solver_->SetFeature(IKSolver::UPDATE_ORIGINAL_POSE, true);
}

void Character::HandleSceneDrawableUpdateFinished(StringHash eventType, VariantMap& eventData)
{
    auto* phyWorld = scene_->GetComponent<PhysicsWorld>();
    Vector3 leftFootPosition = leftFoot_->GetWorldPosition();
    Vector3 rightFootPosition = rightFoot_->GetWorldPosition();

    // Cast ray down to get the normal of the underlying surface
    PhysicsRaycastResult result;
    phyWorld->RaycastSingle(result, Ray(leftFootPosition + Vector3(0, 1, 0), Vector3(0, -1, 0)), 2);
    if (result.body_)
    {
        // Cast again, but this time along the normal. Set the target position
        // to the ray intersection
        phyWorld->RaycastSingle(result, Ray(leftFootPosition + result.normal_, -result.normal_), 2);
        // The foot node has an offset relative to the root node
        float footOffset = leftFoot_->GetWorldPosition().y_ - characterNode_->GetWorldPosition().y_;
        leftEffector_->SetTargetPosition(result.position_ + result.normal_ * footOffset);
        // Rotate foot according to normal
        leftFoot_->Rotate(Quaternion(Vector3(0, 1, 0), result.normal_), TS_WORLD);
    }

    // Same deal with the right foot
    phyWorld->RaycastSingle(result, Ray(rightFootPosition + Vector3(0, 1, 0), Vector3(0, -1, 0)), 2);
    if (result.body_)
    {
        phyWorld->RaycastSingle(result, Ray(rightFootPosition + result.normal_, -result.normal_), 2);
        float footOffset = rightFoot_->GetWorldPosition().y_ - characterNode_->GetWorldPosition().y_;
        rightEffector_->SetTargetPosition(result.position_ + result.normal_ * footOffset);
        rightFoot_->Rotate(Quaternion(Vector3(0, 1, 0), result.normal_), TS_WORLD);
    }

    solver_->Solve();
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
