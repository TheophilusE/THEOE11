
#include "../Input/InputManager.h"
#include "../IO/Log.h"

namespace Urho3D
{
void InputManager::Initialize()
{
    m_InputMap.insert(eastl::pair<eastl::string, InputScheme>("DefaultInputScheme", m_InputScheme));
    LoadSchemes(m_InputMap);
    PushScheme("DefaultInputScheme");
    SetEnabled(true);
}

void InputManager::BuildBindings()
{
    m_InputEventIDNames.clear();
    m_InputEventNameIDs.clear();

    // Condense the scheme stack to reduce redundant work
    eastl::vector<eastl::string> newStack;
    // Removed refrence auto&
    for (auto schemeName = m_CurrentSchemeStack.rbegin(); schemeName != m_CurrentSchemeStack.rend(); schemeName++)
    {
        if (std::find(newStack.begin(), newStack.end(), *schemeName) == newStack.end())
        {
            newStack.push_back(*schemeName);
        }
    }
    m_CurrentSchemeStack = eastl::vector<eastl::string>(newStack.rbegin(), newStack.rend());

    for (auto& scheme : m_CurrentSchemeStack)
    {
        for (auto& boolBinding : m_InputSchemes.at(scheme).bools)
        {
            MapBool_(boolBinding.inputEvent, boolBinding.device, boolBinding.buttons);
        }
        for (auto& floatBinding : m_InputSchemes.at(scheme).floats)
        {
            MapFloat_(floatBinding.inputEvent, floatBinding.device, floatBinding.buttons);
        }
    }
}

void InputManager::SetEnabled_(bool enabled)
{
    if (enabled)
    {
        URHO3D_LOGINFO("Input enabled");
        BuildBindings();
    }
    else
    {
        URHO3D_LOGINFO("Input disabled");
    }
    m_IsEnabled = enabled;
}

void InputManager::LoadSchemes_(const eastl::unordered_map<eastl::string, InputScheme>& inputSchemes)
{
    int count = static_cast<int>(m_InputSchemes.size());
    for (auto& [name, scheme] : inputSchemes)
    {
        AddScheme(name, scheme);
    }

    URHO3D_LOGINFO("Registered {} input scheme (s). Input Registry Size: {} ", inputSchemes.size(), count);
}

void InputManager::AddScheme_(const eastl::string& name, const InputScheme& inputScheme)
{
    if (m_InputSchemes.find(name) != m_InputSchemes.end())
    {
        URHO3D_LOGWARNING("Overriding input scheme: {}", name);
    }
    m_InputSchemes[name] = inputScheme;
}

void InputManager::PopScheme_()
{
    if (m_CurrentSchemeStack.empty())
    {
        URHO3D_LOGWARNING("Input scheme stack is empty. We don't pop anymore.");
        return;
    }

    eastl::string toPop = m_CurrentSchemeStack.back();
    m_CurrentSchemeStack.pop_back();
    BuildBindings();

    URHO3D_LOGINFO("{} input scheme was popped", toPop);
}

void InputManager::FlushSchemes_()
{
    m_InputSchemes.clear();
    m_CurrentSchemeStack.clear();
    BuildBindings();
}

void InputManager::PushScheme_(const eastl::string& name)
{
    if (m_InputSchemes.find(name) == m_InputSchemes.end())
    {
        if (!name.empty())
        {
            URHO3D_LOGWARNING("Could not find a registered input scheme of name: {}", name);
        }
        return;
    }

    m_CurrentSchemeStack.push_back(name);
    BuildBindings();
    URHO3D_LOGINFO("Pushed input scheme: {}", name);
}

void InputManager::MapBool_(const Event::Type& action, Device device, eastl::vector<Key> buttons)
{
    m_InputEventNameIDs[action] = buttons;
    m_InputEventIDNames[m_InputEventNameIDs[action]] = action;
}

void InputManager::MapFloat_(const Event::Type& action, Device device, eastl::vector<Key> buttons)
{
    m_InputEventNameIDs[action] = buttons;
    m_InputEventIDNames[m_InputEventNameIDs[action]] = action;
}

void InputManager::Unmap_(const Event::Type& action) { m_InputEventNameIDs.erase(action); }

bool InputManager::IsPressed_(const Event::Type& action)
{
    if (m_Input)
    {
        auto& keyIDs = m_InputEventNameIDs.at(action);
        for (const auto& iter : keyIDs)
        {
            if (m_IsEnabled && m_Input->GetKeyPress(iter))
            {
                return true;
            }
        }
    }

    return false;
}

bool InputManager::IsDown_(const Event::Type& action)
{
    if (m_Input)
    {
        auto& keyIDs = m_InputEventNameIDs.at(action);
        for (const auto& iter : keyIDs)
        {
            if (m_IsEnabled && m_Input->GetKeyDown(iter))
            {
                return true;
            }
        }
    }

    return false;
}

float InputManager::GetFloat_(const Event::Type& action)
{
    if (m_Input)
    {
        auto& keyIDs = m_InputEventNameIDs.at(action);
        for (const auto& iter : keyIDs)
        {
            // Add Controller Support
            if (m_IsEnabled && m_Input)
            {
                return 0.0f;
            }
        }
    }

    return 0;
}

float InputManager::GetFloatDelta_(const Event::Type& action)
{
    if (m_Input)
    {
        auto& keyIDs = m_InputEventNameIDs.at(action);
        for (const auto& iter : keyIDs)
        {
            // Add Controller Support
            if (m_IsEnabled && m_Input)
            {
                return 0.0f;
            }
        }
    }

    return 0;
}

Vector2 InputManager::GetMousePosition_()
{
    if (m_Input)
        return {static_cast<float>(m_Input->GetMousePosition().x_), static_cast<float>(m_Input->GetMousePosition().y_)};

    return Vector2();
}

bool InputManager::MouseButtonDown_(MouseButtonFlags button) { return m_Input->GetMouseButtonDown(button); }
bool InputManager::MouseButtonPressed_(MouseButtonFlags button) { return m_Input->GetMouseButtonPress(button); }
bool InputManager::MouseButtonClicked_(MouseButtonFlags button) { return m_Input->GetMouseButtonClick(button); }
int InputManager::GetMouseMoveWheel_() { return m_Input->GetMouseMoveWheel(); };
IntVector2 InputManager::GetMouseMove_() { return m_Input->GetMouseMove(); }
bool InputManager::IsMouseVisible_() { return m_Input->IsMouseVisible(); }
bool InputManager::IsMouseGrabbed_() { return m_Input->IsMouseGrabbed(); }
bool InputManager::IsMouseLocked_() { return m_Input->IsMouseLocked(); }
MouseMode InputManager::GetMouseMode_() { return m_Input->GetMouseMode(); }
void InputManager::SetMousePosition_(const IntVector2& position) { return m_Input->SetMousePosition(position); }
void InputManager::CenterMousePosition_() { return m_Input->CenterMousePosition(); }
void InputManager::SetMouseMode_(MouseMode mode) { return m_Input->SetMouseMode(mode); }
void InputManager::ResetMouseMode_() { return m_Input->ResetMouseMode(); }
void InputManager::SetToggleFullscreen_(bool enable) { return m_Input->SetToggleFullscreen(enable); }
void InputManager::SetMouseVisible_(bool enable) { return m_Input->SetMouseVisible(enable); }
void InputManager::ResetMouseVisible_() { return m_Input->ResetMouseVisible(); }
void InputManager::SetMouseGrabbed_(bool grab) { return m_Input->SetMouseGrabbed(grab); }
void InputManager::ResetMouseGrabbed_() { return m_Input->ResetMouseGrabbed(); }
bool InputManager::HasFocus_() { return m_Input->HasFocus(); }
bool InputManager::IsMinimized_() { return m_Input->IsMinimized(); }

InputManager::InputManager()
    : m_IsEnabled(true)
{
}

InputManager* InputManager::GetSingleton()
{
    static InputManager singleton;
    return &singleton;
}

} // namespace Urho3D
