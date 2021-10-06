
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Math/Ray.h>
#include <Urho3D/UI/UI.h>
#include "PlayerCameraManager.h"
#include "../Character/BaseCharacter.h"

namespace Urho3D
{
void PlayerCameraManager::Update(float timeStep)
{
    Input* input = context_->GetSubsystem<Input>();
    Character* character = GetNode()->GetComponent<Character>();

    if (character)
    {
        // Clear previous controls
        character->controls_.Set(CTRL_FORWARD | CTRL_BACK | CTRL_LEFT | CTRL_RIGHT | CTRL_JUMP, false);

        // Update controls using keys
        auto* ui = GetSubsystem<UI>();
        if (!ui->GetFocusElement())
        {
            {
                character->controls_.Set(CTRL_FORWARD, input->GetKeyDown(KEY_W));
                character->controls_.Set(CTRL_BACK, input->GetKeyDown(KEY_S));
                character->controls_.Set(CTRL_LEFT, input->GetKeyDown(KEY_A));
                character->controls_.Set(CTRL_RIGHT, input->GetKeyDown(KEY_D));
            }
            character->controls_.Set(CTRL_JUMP, input->GetKeyDown(KEY_SPACE));

            {
                character->controls_.yaw_ += (float)input->GetMouseMoveX() * YAW_SENSITIVITY;
                character->controls_.pitch_ += (float)input->GetMouseMoveY() * YAW_SENSITIVITY;
            }
            // Limit pitch
            character->controls_.pitch_ = Clamp(character->controls_.pitch_, -80.0f, 80.0f);
            // Set rotation already here so that it's updated every rendering frame instead of every physics frame
            character->GetNode()->SetRotation(Quaternion(character->controls_.yaw_, Vector3::UP));

            // Switch between 1st and 3rd person
            if (input->GetKeyPress(KEY_F))
                m_Camera.firstPerson = !m_Camera.firstPerson;
        }
    }
}

void PlayerCameraManager::PostUpdate(float timeStep)
{
    Character* character = GetNode()->GetComponent<Character>();

    if (!character)
        return;

    Node* characterNode = character->GetNode();

    // Get camera lookat dir from character yaw + pitch
    const Quaternion& rot = characterNode->GetRotation();
    Quaternion dir = rot * Quaternion(character->controls_.pitch_, Vector3::RIGHT);

    // Turn head to camera pitch, but limit to avoid unnatural animation
    Node* headNode = characterNode->GetChild("Mutant:Head", true);
    float limitPitch = Clamp(character->controls_.pitch_, -45.0f, 45.0f);
    Quaternion headDir = rot * Quaternion(limitPitch, Vector3(1.0f, 0.0f, 0.0f));
    // This could be expanded to look at an arbitrary target, now just look at a point in front
    Vector3 headWorldTarget = headNode->GetWorldPosition() + headDir * Vector3(0.0f, 0.0f, -1.0f);
    headNode->LookAt(headWorldTarget, Vector3(0.0f, 1.0f, 0.0f));

    if (m_Camera.firstPerson)
    {
        GetNode()->SetPosition(headNode->GetWorldPosition() + rot * Vector3(0.0f, 0.15f, 0.2f));
        GetNode()->SetRotation(dir);
    }
    else
    {
        // Third person camera: position behind the character
        Vector3 aimPoint = characterNode->GetPosition() + rot * Vector3(0.0f, 1.7f, 0.0f);

        // Collide camera ray with static physics objects (layer bitmask 2) to ensure we see the character properly
        Vector3 rayDir = dir * Vector3::BACK;
        float rayDistance = m_Camera.initialDistance;
        PhysicsRaycastResult result;
        GetNode()->GetParent()->GetComponent<PhysicsWorld>()->RaycastSingle(result, Ray(aimPoint, rayDir), rayDistance, 2);
        if (result.body_)
            rayDistance = Min(rayDistance, result.distance_);
        rayDistance = Clamp(rayDistance, m_Camera.minDistance, m_Camera.maxDistance);

        GetNode()->SetPosition(aimPoint + rayDir * rayDistance);
        GetNode()->SetRotation(dir);
    }
}

void PlayerCameraManager::RegisterObject(Context* context, PluginApplication* plugin)
{
    plugin->RegisterFactory<PlayerCameraManager>("STAR Plugin");

    /// Add Macros Here
    URHO3D_ATTRIBUTE("Initial Distance", float, m_Camera.initialDistance, 5.0f, AM_DEFAULT);
    URHO3D_ATTRIBUTE("Min Distance", float, m_Camera.minDistance, 1.0f, AM_DEFAULT);
    URHO3D_ATTRIBUTE("Max Distance", float, m_Camera.maxDistance, 1.0f, AM_DEFAULT);
}
}
