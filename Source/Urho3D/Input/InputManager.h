//
// Copyright (c) 2008-2020 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include "../Core/Object.h"
#include "../Engine/Event.h"
#include "../Input/Input.h"

namespace Urho3D
{

/// Enumeration of the devices supported. Any new device can be added here.
enum class URHO3D_API Device
{
    Mouse,
    Keyboard
};

/// Input Layout Description
struct URHO3D_API InputDescription
{
    Device device;
    eastl::vector<Key> buttons;
    Event::Type inputEvent;
};

struct URHO3D_API InputScheme
{
    eastl::vector<InputDescription> bools;
    eastl::vector<InputDescription> floats;
};

/// Allows interfacing to game controlling hardware, including mouse, keyboard and XInput controllers.
/// Allows detecting inputs through Event dispatch.
/// Event data for boolean buttons consists of a Vector2 where Vector2.x and Vector2.y carry the old and new values for
/// the input event respectively. Float buttons should be queried directly by invoking InputManager.
class URHO3D_API InputManager
{
    bool m_IsEnabled;
    eastl::unordered_map<Device, eastl::vector<Key>> DeviceIDs;
    eastl::unordered_map<eastl::string, InputScheme> m_InputSchemes;
    eastl::vector<eastl::string> m_CurrentSchemeStack;

    eastl::unordered_map<eastl::vector<Key>, Event::Type> m_InputEventIDNames;
    eastl::unordered_map<Event::Type, eastl::vector<Key>> m_InputEventNameIDs;

    InputManager();
    InputManager(InputManager&) = delete;
    ~InputManager() = default;

    void BuildBindings();

public:
    static InputManager* GetSingleton();
    static void SetEnabled(bool enabled) { GetSingleton()->SetEnabled_(enabled); };
    static void MapBool(const Event::Type& action, Device device, eastl::vector<Key> buttons)
    {
        GetSingleton()->MapBool_(action, device, buttons);
    };
    static void MapFloat(const Event::Type& action, Device device, eastl::vector<Key> buttons)
    {
        GetSingleton()->MapFloat_(action, device, buttons);
    };
    static bool IsPressed(const Event::Type& action) { return GetSingleton()->IsPressed_(action); };
    static bool IsDown(const Event::Type& action) { return GetSingleton()->IsDown_(action); };
    static float GetFloat(const Event::Type& action) { return GetSingleton()->GetFloat_(action); };
    static float GetFloatDelta(const Event::Type& action) { return GetSingleton()->GetFloatDelta_(action); };
    static void Unmap(const Event::Type& action) { GetSingleton()->Unmap_(action); };
    static void LoadSchemes(const eastl::unordered_map<eastl::string, InputScheme>& inputSchemes)
    {
        return GetSingleton()->LoadSchemes_(inputSchemes);
    }
    static void AddScheme(const eastl::string& name, const InputScheme& inputScheme)
    {
        return GetSingleton()->AddScheme_(name, inputScheme);
    }
    static void PushScheme(const eastl::string& schemeName) { return GetSingleton()->PushScheme_(schemeName); }
    static void PopScheme() { return GetSingleton()->PopScheme_(); }
    static void FlushSchemes() { return GetSingleton()->FlushSchemes_(); }
    static Vector2 GetMousePosition() { return GetSingleton()->GetMousePosition_(); };
    static bool MouseButtonDown(MouseButtonFlags button) { return GetSingleton()->MouseButtonDown_(button); }
    static bool MouseButtonPressed(MouseButtonFlags button) { return GetSingleton()->MouseButtonPressed_(button); }
    static bool MouseButtonClicked(MouseButtonFlags button) { return GetSingleton()->MouseButtonClicked_(button); }
    static IntVector2 GetMouseMove() { return GetSingleton()->GetMouseMove_(); }
    static int GetMouseMoveWheel() { return GetSingleton()->GetMouseMoveWheel_(); }
    static bool IsMouseVisible() { return GetSingleton()->IsMouseVisible_(); }
    static bool IsMouseGrabbed() { return GetSingleton()->IsMouseGrabbed_(); }
    static bool IsMouseLocked() { return GetSingleton()->IsMouseLocked_(); }
    static MouseMode GetMouseMode() { return GetSingleton()->GetMouseMode_(); }
    static void SetMousePosition(const IntVector2& position) { return GetSingleton()->SetMousePosition_(position); }
    static void CenterMousePosition() { return GetSingleton()->CenterMousePosition_(); }
    static void SetMouseMode(MouseMode mode) { return GetSingleton()->SetMouseMode_(mode); }
    static void ResetMouseMode() { return GetSingleton()->ResetMouseMode_(); }
    static void SetToggleFullscreen(bool enable) { return GetSingleton()->SetToggleFullscreen_(enable); }
    static void SetMouseVisible(bool enable) { return GetSingleton()->SetMouseVisible_(enable); }
    static void ResetMouseVisible() { return GetSingleton()->ResetMouseVisible_(); }
    static void SetMouseGrabbed(bool grab) { return GetSingleton()->SetMouseGrabbed_(grab); }
    static void ResetMouseGrabbed() { return GetSingleton()->ResetMouseGrabbed_(); }
    static bool HasFocus() { return GetSingleton()->HasFocus_(); }
    static bool IsMinimized() { return GetSingleton()->IsMinimized_(); }

private:
    void Initialize();

    void SetEnabled_(bool enabled);

    void LoadSchemes_(const eastl::unordered_map<eastl::string, InputScheme>& inputSchemes);
    void AddScheme_(const eastl::string& name, const InputScheme& inputScheme);
    void PushScheme_(const eastl::string& schemeName);
    void PopScheme_();
    void FlushSchemes_();

    /// Bind an event to a button on a device.
    void MapBool_(const Event::Type& action, Device device, eastl::vector<Key> buttons);
    /// Bind an event to a float on a device.
    void MapFloat_(const Event::Type& action, Device device, eastl::vector<Key> buttons);

    void Unmap_(const Event::Type& action);

    /// KeyBoard
    bool IsPressed_(const Event::Type& action);
    bool IsDown_(const Event::Type& action);

    /// Controller / Joystick
    float GetFloat_(const Event::Type& action);
    float GetFloatDelta_(const Event::Type& action);

    /// Mouse
    Vector2 GetMousePosition_();
    bool MouseButtonDown_(MouseButtonFlags button);
    bool MouseButtonPressed_(MouseButtonFlags button);
    bool MouseButtonClicked_(MouseButtonFlags button);
    IntVector2 GetMouseMove_();
    int GetMouseMoveWheel_();
    bool IsMouseVisible_();
    bool IsMouseGrabbed_();
    bool IsMouseLocked_();
    MouseMode GetMouseMode_();
    void SetMousePosition_(const IntVector2& position);
    void CenterMousePosition_();
    void SetMouseMode_(MouseMode mode);
    void ResetMouseMode_();
    void SetToggleFullscreen_(bool enable);
    void SetMouseVisible_(bool enable);
    void ResetMouseVisible_();
    void SetMouseGrabbed_(bool grab);
    void ResetMouseGrabbed_();

    bool HasFocus_();
    bool IsMinimized_();

public:
    WeakPtr<Input> m_Input;
    /// Store Input Schemes
    eastl::unordered_map<eastl::string, InputScheme> m_InputMap;
    /// Store Key and Keybind Name
    InputScheme m_InputScheme;
};
} // namespace Urho3D
