
#pragma once

#include <Urho3D/Engine/Application.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/SystemUI/Console.h>
#include <Urho3D/UI/Cursor.h>
#if URHO3D_SYSTEMUI
#include <Urho3D/SystemUI/DebugHud.h>
#endif
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/UI/Sprite.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Core/Timer.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Core/Profiler.h>


// All Urho3D classes reside in namespace Urho3D
using namespace Urho3D;


const float TOUCH_SENSITIVITY = 2.0f;

/// App class, as framework for all Apps.
///    - Initialization of the Urho3D engine (in Application class)
///    - Modify engine parameters for windowed mode and to show the class name as title
///    - Create Urho3D logo at screen
///    - Set custom window title and icon
///    - Create Console and Debug HUD, and use F1 and F2 key to toggle them
///    - Toggle rendering options from the keys 1-8
///    - Take screenshot with key 9
///    - Handle Esc key down to hide Console or exit application
///    - Init touch input on mobile platform using screen joysticks (patched for each individual App)

class App : public Application
{
    // Enable type information.
    URHO3D_OBJECT(App, Object);

public:
    /// Construct.
    explicit App(Context* context);

    /// Setup before engine initialization. Modifies the engine parameters.
    void Setup() override;
    /// Setup after engine initialization. Creates the logo, console & debug HUD.
    void Start() override;
    /// Cleanup after the main loop. Called by Application.
    void Stop() override;

protected:
    /// Return XML patch instructions for screen joystick layout for a specific App, if any.
    virtual ea::string GetScreenJoystickPatchString() const { return EMPTY_STRING; }
    /// Initialize touch input on mobile platform.
    void InitTouchInput();
    /// Initialize mouse mode on non-web platform.
    void InitMouseMode(MouseMode mode);
    /// Control logo visibility.
    void SetLogoVisible(bool enable);
    ///
    void CloseApp();

    /// Logo sprite.
    SharedPtr<Sprite> logoSprite_;
    /// Scene.
    SharedPtr<Scene> scene_;
    /// Camera scene node.
    SharedPtr<Node> cameraNode_;
    /// Camera yaw angle.
    float yaw_;
    /// Camera pitch angle.
    float pitch_;
    /// Flag to indicate whether touch input has been enabled.
    bool touchEnabled_;
    /// Mouse mode option to use in the App.
    MouseMode useMouseMode_;

private:
    /// Create logo.
    void CreateLogo();
    /// Set custom window Title & Icon
    void SetWindowTitleAndIcon();
    /// Create console and debug HUD.
    void CreateConsoleAndDebugHud();
    /// Handle request for mouse mode on web platform.
    void HandleMouseModeRequest(StringHash eventType, VariantMap& eventData);
    /// Handle request for mouse mode change on web platform.
    void HandleMouseModeChange(StringHash eventType, VariantMap& eventData);
    /// Handle key down event to process key controls common to all Apps.
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
    /// Handle key up event to process key controls common to all Apps.
    void HandleKeyUp(StringHash eventType, VariantMap& eventData);
    /// Handle scene update event to control camera's pitch and yaw for all Apps.
    void HandleSceneUpdate(StringHash eventType, VariantMap& eventData);
    /// Handle touch begin event to initialize touch input on desktop platform.
    void HandleTouchBegin(StringHash eventType, VariantMap& eventData);

    /// Screen joystick index for navigational controls (mobile platforms only).
    unsigned screenJoystickIndex_;
    /// Screen joystick index for settings (mobile platforms only).
    unsigned screenJoystickSettingsIndex_;
    /// Pause flag.
    bool paused_;
};
