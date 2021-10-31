
#pragma once

#include <Urho3D/Input/Controls.h>
#include <Urho3D/Scene/LogicComponent.h>

using namespace Urho3D;

const float WALK_FORCE = 0.5f;
const float RUN_FORCE = 0.8f;
const float SPRINT_FORCE = 1.6f;
const float INAIR_MOVE_FORCE = 0.02f;
const float BRAKE_FORCE = 0.2f;
const float JUMP_FORCE = 7.0f;
const float YAW_SENSITIVITY = 0.1f;
const float INAIR_THRESHOLD_TIME = 0.1f;

namespace Urho3D
{
class AnimationController;
class Node;
class IKEffector;
class IKSolver;
class Scene;

/// Character component, responsible for physical movement according to controls, as well as animation.
class Character : public LogicComponent
{
    URHO3D_OBJECT(Character, LogicComponent);

public:
    /// Construct.
    explicit Character(Context* context);

    /// Register object factory and attributes.
    static void RegisterObject(Context* context);

    /// Handle startup. Called by LogicComponent base class.
    void Start() override;
    /// Handle physics world update. Called by LogicComponent base class.
    void FixedUpdate(float timeStep) override;

    /// Do physics Update for Stepping
    void DoPhysicsStep(float timeStep);
    /// Draw debug geometry.
    void DrawDebug();
    /// Handle Setup
    void Setup();
    /// Handle IK Solve
    void HandleSceneDrawableUpdateFinished(StringHash eventType, VariantMap& eventData);

    /// Scene.
    SharedPtr<Scene> scene_;

private:
    /// Handle physics collision event.
    void HandleNodeCollision(StringHash eventType, VariantMap& eventData);

    /// Grounded flag for movement.
    bool onGround_;
    /// Jump flag.
    bool okToJump_;
    /// In air timer. Due to possible physics inaccuracy, character can be off ground for max. 1/10 second and still be
    /// allowed to move.
    float inAirTimer_;

    bool isWalk;
    bool isSprint;

    // Physics Stepping for inclined planes, stairs / steps, obstacles etc.
    float stepHeight = 0.3f;
    float stepSmooth = 2.f;
    SharedPtr<Node> stepRayUpper;
    SharedPtr<Node> stepRayLower;

    /// Inverse kinematic left effector.
    SharedPtr<IKEffector> leftEffector_;
    /// Inverse kinematic right effector.
    SharedPtr<IKEffector> rightEffector_;
    /// Inverse kinematic solver.
    SharedPtr<IKSolver> solver_;
    /// Need references to these nodes to calculate foot angles and offsets.
    SharedPtr<Node> leftFoot_;
    SharedPtr<Node> rightFoot_;
    SharedPtr<Node> characterNode_;
};

} // namespace Urho3D
