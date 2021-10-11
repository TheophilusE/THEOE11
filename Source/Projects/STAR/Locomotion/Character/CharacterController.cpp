
#include "../Character/BaseCharacter.h"
#include "CharacterController.h"
#include <Urho3D/IO/Log.h>
#include <Urho3D/Math/Ray.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneManager.h>
#include <Urho3D/UI/UI.h>

namespace Urho3D
{
void CharacterController::Start()
{
    character_ = GetNode()->GetComponent<Character>(true);
}

void CharacterController::Update(float timeStep)
{
    auto* input = GetSubsystem<Input>();

    if (character_)
    {
        // Clear previous controls
        character_->controls_.Set(CTRL_FORWARD | CTRL_BACK | CTRL_LEFT | CTRL_RIGHT | CTRL_JUMP, false);

        // Update controls using keys
        auto* ui = GetSubsystem<UI>();
        if (!ui->GetFocusElement())
        {
            {
                character_->controls_.Set(CTRL_FORWARD, input->GetKeyDown(KEY_W));
                character_->controls_.Set(CTRL_BACK, input->GetKeyDown(KEY_S));
                character_->controls_.Set(CTRL_LEFT, input->GetKeyDown(KEY_A));
                character_->controls_.Set(CTRL_RIGHT, input->GetKeyDown(KEY_D));
                character_->controls_.Set(CTRL_JUMP, input->GetKeyDown(KEY_SPACE));
                character_->controls_.Set(CTRL_WALK_RUN, input->GetKeyPress(KEY_ALT));
                character_->controls_.Set(CTRL_SPRINT, input->GetKeyDown(KEY_SHIFT));
            }

            {
                character_->controls_.yaw_ += (float)input->GetMouseMoveX() * YAW_SENSITIVITY;
                character_->controls_.pitch_ += (float)input->GetMouseMoveY() * YAW_SENSITIVITY;
            }
            // Limit pitch
            character_->controls_.pitch_ = Clamp(character_->controls_.pitch_, -80.0f, 80.0f);
            // Set rotation already here so that it's updated every rendering frame instead of every physics frame
            character_->GetNode()->SetRotation(Quaternion(character_->controls_.yaw_, Vector3::UP));
        }
    }
}

void CharacterController::PostUpdate(float timeStep)
{
    if (!character_)
        return;
}

void CharacterController::RegisterObject(Context* context, PluginApplication* plugin)
{
    plugin->RegisterFactory<CharacterController>("STAR Plugin");
}

void CharacterController::RegisterObject(Context* context)
{
    context->RegisterFactory<CharacterController>();
}

} // namespace Urho3D
