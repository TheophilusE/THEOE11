
#include "../Character/BaseCharacter.h"
#include "CharacterController.h"
#include <Urho3D/IO/Log.h>
#include <Urho3D/Math/Ray.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneManager.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Input/InputManager.h>

namespace Urho3D
{
void CharacterController::Start()
{
    character_ = GetNode()->GetComponent<Character>(true);

    InputDescription a;
    a.device = Device::Keyboard;
    a.buttons = {Key::KEY_W};
    a.inputEvent = "MoveForward";

    InputDescription b;
    b.device = Device::Keyboard;
    b.buttons = {Key::KEY_S};
    b.inputEvent = "MoveBackward";

    InputDescription c;
    c.device = Device::Keyboard;
    c.buttons = {Key::KEY_D};
    c.inputEvent = "MoveRight";

    InputDescription d;
    d.device = Device::Keyboard;
    d.buttons = {Key::KEY_A};
    d.inputEvent = "MoveLeft";

    InputDescription e;
    e.device = Device::Keyboard;
    e.buttons = {Key::KEY_UNKNOWN};
    e.inputEvent = "Und";

    InputDescription f;
    f.device = Device::Keyboard;
    f.buttons = {Key::KEY_SPACE};
    f.inputEvent = "Jump";

    InputDescription g;
    g.device = Device::Keyboard;
    g.buttons = {Key::KEY_ALT};
    g.inputEvent = "ToggleWalk";

    InputDescription h;
    h.device = Device::Keyboard;
    h.buttons = {Key::KEY_SHIFT};
    h.inputEvent = "ToggleSprint";

    InputManager::GetSingleton()->m_InputScheme.bools.push_back(a);
    InputManager::GetSingleton()->m_InputScheme.bools.push_back(b);
    InputManager::GetSingleton()->m_InputScheme.bools.push_back(c);
    InputManager::GetSingleton()->m_InputScheme.bools.push_back(d);
    InputManager::GetSingleton()->m_InputScheme.bools.push_back(e);
    InputManager::GetSingleton()->m_InputScheme.bools.push_back(f);
    InputManager::GetSingleton()->m_InputScheme.bools.push_back(g);
    InputManager::GetSingleton()->m_InputScheme.bools.push_back(h);

    InputManager::GetSingleton()->m_InputMap.insert(eastl::pair<eastl::string, InputScheme>(
        InputManager::GetSingleton()->m_StartScheme, InputManager::GetSingleton()->m_InputScheme));

    InputManager::LoadSchemes(InputManager::GetSingleton()->m_InputMap);
    InputManager::PushScheme(InputManager::GetSingleton()->m_StartScheme);
    InputManager::SetEnabled(true);

    InputManager::SaveInputMapToFile("Config/InputMap.json");
    InputManager::LoadInputMapFromFile("Config/InputMap.json");
}

void CharacterController::Update(float timeStep)
{

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
