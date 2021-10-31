
#include "../Input/InputManager.h"
#include "../Engine/Engine.h"
#include "../Graphics/Graphics.h"
#include "../IO/Archive.h"
#include "../IO/ArchiveSerialization.h"
#include "../IO/FileSystem.h"
#include "../IO/Log.h"
#include "../Resource/JSONArchive.h"
#include "../Resource/ResourceCache.h"
#include "../Resource/ResourceEvents.h"

namespace Urho3D
{
void InputManager::Initialize_() { SetEnabled(true); }

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

bool InputManager::SaveInputMapToFile_(eastl::string filePath)
{
    auto* cache = m_Input->GetSubsystem<ResourceCache>();
    filePath = (cache->GetResourceDirs()[0] + filePath);

    JSONFile iMapFile(m_Input->GetContext());
    JSONOutputArchive archive(&iMapFile);

    URHO3D_LOGINFO("Serializing InputMap to File at {}", filePath);
    if (!SerializeInput_(archive, InputManager::GetSingleton()->m_InputMap))
    {
        URHO3D_LOGINFO("failed to serialize archive");
        return false;
    }
    URHO3D_LOGINFO("Archive Has Error: {}", archive.HasError());

    File file(m_Input->GetContext(), filePath, FILE_WRITE);
    iMapFile.Save(file);

    return true;
}

bool InputManager::LoadInputMapFromFile_(eastl::string filePath)
{
    JSONFile file(m_Input.Get()->GetContext());
    auto* cache = m_Input.Get()->GetSubsystem<ResourceCache>();
    filePath = (cache->GetResourceDirs()[0] + filePath);

    URHO3D_LOGINFO("Deserializing InputMap from File at {}", filePath);
    if (file.LoadFile(filePath))
    {
        DeserializeInput_(file, InputManager::GetSingleton()->m_InputMap);

        InputManager::LoadSchemes(InputManager::GetSingleton()->m_InputMap);
        InputManager::PushScheme(m_StartScheme);
        InputManager::SetEnabled(true);
    }
    else
        URHO3D_LOGINFO("Failed to load file at {}", filePath);

    return true;
}

bool InputManager::SerializeInput_(Archive& archive, eastl::unordered_map<eastl::string, InputScheme>& inputMaps)
{
    const int version = 1;
    if (!archive.IsInput() && m_Input.Get()->GetContext()->GetSubsystem<Engine>()->IsHeadless())
    {
        URHO3D_LOGERROR("Headless instance is supposed to use project as read-only.");
        return false;
    }

    if (auto projectBlock = archive.OpenUnorderedBlock("InputRegistry"))
    {
        int archiveVersion = version;
        SerializeValue(archive, "version", archiveVersion);

        if (auto projectBlock = archive.OpenUnorderedBlock("inputSchemes"))
        {
            for (auto& iter1 : inputMaps)
            {
                if (auto projectBlock = archive.OpenUnorderedBlock(iter1.first.c_str()))
                {
                    if (auto projectBlock = archive.OpenSequentialBlock("bools"))
                    {
                        for (auto& iter2a : iter1.second.bools)
                        {
                            if (auto projectBlock = archive.OpenUnorderedBlock(""))
                            {
                                int device = (int)iter2a.device;
                                int buttonSize = iter2a.buttons.size();

                                SerializeValue(archive, "device", device);
                                SerializeValue(archive, "inputEvent", iter2a.inputEvent);

                                bool multiButton{true};
                                int count = 0;
                                if (multiButton)
                                {
                                    SerializeValue(archive, "buttonSize", buttonSize);
                                    for (auto& iter2buttons : iter2a.buttons)
                                    {
                                        int button = (int)iter2buttons;
                                        SerializeValue(archive, eastl::string{fmt::format("button_{}", count)}.c_str(),
                                                       button);
                                        count++;
                                    }
                                }
                                else
                                {
                                    int button = (int)iter2a.buttons[0];
                                    SerializeValue(archive, eastl::string{fmt::format("button_{}", count)}.c_str(),
                                                   button);
                                }
                            }
                        }
                    }

                    if (auto projectBlock = archive.OpenSequentialBlock("floats"))
                    {
                        for (auto& iter2a : iter1.second.floats)
                        {
                            if (auto projectBlock = archive.OpenUnorderedBlock(""))
                            {
                                if (auto projectBlock = archive.OpenUnorderedBlock(""))
                                {
                                    int device = (int)iter2a.device;
                                    int buttonSize = iter2a.buttons.size();

                                    SerializeValue(archive, "device", device);
                                    SerializeValue(archive, "inputEvent", iter2a.inputEvent);

                                    bool multiButton{true};
                                    int count = 0;
                                    if (multiButton)
                                    {
                                        SerializeValue(archive, "buttonSize", buttonSize);
                                        for (auto& iter2buttons : iter2a.buttons)
                                        {
                                            int button = (int)iter2buttons;
                                            SerializeValue(archive,
                                                           eastl::string{fmt::format("button_{}", count)}.c_str(),
                                                           button);
                                            count++;
                                        }
                                    }
                                    else
                                    {
                                        int button = (int)iter2a.buttons[0];
                                        SerializeValue(archive, eastl::string{fmt::format("button_{}", count)}.c_str(),
                                                       button);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return true;
}

bool InputManager::DeserializeInput_(JSONFile& file, eastl::unordered_map<eastl::string, InputScheme>& inputMaps)
{
    inputMaps.clear();
    auto& root = file.GetRoot();

    URHO3D_LOGINFO("Root Size {}", root.GetObject().size());
    URHO3D_LOGINFO("Scheme Size {}", root["inputSchemes"].Size());

    for (auto& iter1 : root["inputSchemes"][m_StartScheme])
    {
        InputScheme scheme;

        auto& bools = iter1.second.GetArray();
        auto& inputDesc = bools;
        URHO3D_LOGINFO("Input Array Size: {}", bools.size());

        for (auto& iter : inputDesc)
        {
            InputDescription a;
            a.device = (Device)iter.Get("device").GetInt();
            a.inputEvent = iter.Get("inputEvent").GetString();
            int bSize = iter.Get("buttonSize").GetInt();

            eastl::vector<Key> iMap;
            for (int i = 0; i < bSize; i++)
            {
                iMap.push_back((Key)iter.Get(fmt::format("button_{}", i)).GetInt());
            }
            a.buttons = iMap;
            scheme.bools.push_back(a);
        }

        for (auto& m : scheme.bools)
        {
            URHO3D_LOGINFO("-------------------------------");
            URHO3D_LOGINFO("InputDevice: {}", (int)m.device);
            URHO3D_LOGINFO("InputEvents, {}", m.inputEvent);

            for (auto t : m.buttons)
            {
                URHO3D_LOGINFO("Keys: {}", (int)t);
            }
            URHO3D_LOGINFO("-------------------------------");
        }

        inputMaps.insert(eastl::pair<eastl::string, InputScheme>(m_StartScheme, scheme));
    }

    return true;
}

InputManager::InputManager()
    : m_IsEnabled(true)
    , m_StartScheme("DefaultInputScheme")
{
}

InputManager* InputManager::GetSingleton()
{
    static InputManager singleton;
    return &singleton;
}

} // namespace Urho3D
