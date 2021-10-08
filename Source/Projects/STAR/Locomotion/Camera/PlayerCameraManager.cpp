
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Math/Ray.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneManager.h>
#include "PlayerCameraManager.h"
#include "../Character/BaseCharacter.h"

namespace Urho3D
{
void PlayerCameraManager::Start()
{
    character_ = GetNode()->GetComponent<Character>(true);
}

void PlayerCameraManager::Update(float timeStep)
{
    auto* input = GetSubsystem<Input>();

    if (character_)
    {
        // Switch between 1st and 3rd person
        if (input->GetKeyPress(KEY_F))
        {
            m_Camera.firstPerson = !m_Camera.firstPerson;
        }
    }
}

void PlayerCameraManager::PostUpdate(float timeStep)
{
    if (!character_)
        return;

    Node* characterNode = character_->GetNode();

    // Get camera lookat dir from character yaw + pitch
    const Quaternion& rot = characterNode->GetRotation();
    Quaternion dir = rot * Quaternion(character_->controls_.pitch_, Vector3::RIGHT);

    // Turn head to camera pitch, but limit to avoid unnatural animation
    Node* headNode = characterNode->GetChild("head", true);
    float limitPitch = Clamp(character_->controls_.pitch_, -45.0f, 45.0f);
    Quaternion headDir = rot * Quaternion(limitPitch, Vector3(1.0f, 0.0f, 0.0f));
    // This could be expanded to look at an arbitrary target, now just look at a point in front
    Vector3 headWorldTarget = headNode->GetWorldPosition() + headDir * Vector3(0.0f, 0.0f, -1.0f);
    // headNode->LookAt(headWorldTarget, Vector3(0.0f, 1.0f, 0.0f));

    if (m_Camera.firstPerson)
    {
        cameraNode_->SetPosition(headNode->GetWorldPosition() + rot * Vector3(0.0f, 0.15f, 0.2f));
        cameraNode_->SetRotation(dir);
    }
    else
    {
        // Third person camera: position behind the character
        Vector3 aimPoint = characterNode->GetPosition() + rot * Vector3(0.0f, 1.7f, 0.0f);

        // Collide camera ray with static physics objects (layer bitmask 2) to ensure we see the character properly
        Vector3 rayDir = dir * Vector3::BACK;
        float rayDistance = m_Camera.initialDistance;
        PhysicsRaycastResult result;
        scene_->GetComponent<PhysicsWorld>()->RaycastSingle(result, Ray(aimPoint, rayDir), rayDistance, 2);
        if (result.body_)
            rayDistance = Min(rayDistance, result.distance_);
        rayDistance = Clamp(rayDistance, m_Camera.minDistance, m_Camera.maxDistance);

        cameraNode_->SetPosition(aimPoint + rayDir * rayDistance);
        cameraNode_->SetRotation(dir);
    }
}

void PlayerCameraManager::RegisterObject(Context* context, PluginApplication* plugin)
{
    plugin->RegisterFactory<PlayerCameraManager>("STAR Plugin");

    /// Add Macros Here
    URHO3D_ATTRIBUTE("Initial Distance", float, m_Camera.initialDistance, 5.0f, AM_DEFAULT);
    URHO3D_ATTRIBUTE("Min Distance", float, m_Camera.minDistance, 1.0f, AM_DEFAULT);
    URHO3D_ATTRIBUTE("Max Distance", float, m_Camera.maxDistance, 20.0f, AM_DEFAULT);
}

void PlayerCameraManager::RegisterObject(Context* context)
{
    context->RegisterFactory<PlayerCameraManager>();

    /// Add Macros Here
    URHO3D_ATTRIBUTE("Initial Distance", float, m_Camera.initialDistance, 5.0f, AM_DEFAULT);
    URHO3D_ATTRIBUTE("Min Distance", float, m_Camera.minDistance, 1.0f, AM_DEFAULT);
    URHO3D_ATTRIBUTE("Max Distance", float, m_Camera.maxDistance, 20.0f, AM_DEFAULT);
}

void PlayerCameraManager::SetUp(Scene* scene, Node* cameraNode)
{
    scene_ = scene;
    cameraNode_ = cameraNode;
}

}
