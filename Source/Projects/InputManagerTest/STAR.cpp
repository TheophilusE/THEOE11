
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Input/Controls.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputManager.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>

#include "STAR.h"

#include <Urho3D/DebugNew.h>

/*
 * Improvements
 * - Add checks for begining and end before executing the search
 * - Use Sorting algorithms
 * - Add Controller Support to InputManager
 * - Add Serializing
 */

void InputManagerTest::Setup()
{
    App::Setup();

    // Engine is not initialized yet. Set up all the parameters now.
    engineParameters_[EP_WINDOW_TITLE] = "InputManagerTest";
    engineParameters_[EP_VSYNC] = true;
    engineParameters_[EP_WINDOW_MAXIMIZE] = true;
    engineParameters_[EP_FULL_SCREEN] = false;
    engineParameters_[EP_WINDOW_HEIGHT] = 720;
    engineParameters_[EP_WINDOW_WIDTH] = 1280;
    engineParameters_[EP_RESOURCE_PATHS] = "C:\\THEO\\Projects\\EngineTest";
}

void InputManagerTest::Start()
{
    // At this point engine is initialized, but first frame was not rendered yet. Further setup should be done here. To
    // make sample a little bit user friendly show mouse cursor here.
    App::Start();

    GetSubsystem<Input>()->SetMouseVisible(true);

    // Subscribe to necessary events
    SubscribeToEvents();

    // Add Input Keys
    InputDescription a;
    a.device = Device::Keyboard;
    a.buttons = {Key::KEY_SPACE, Key::KEY_UP, Key::KEY_0};
    a.inputEvent = "LongPress";

    InputDescription b;
    b.device = Device::Keyboard;
    b.buttons = {Key::KEY_W, Key::KEY_A, Key::KEY_S, Key::KEY_D};
    b.inputEvent = "OnePress";

    InputDescription c;
    c.device = Device::Keyboard;
    c.buttons = {Key::KEY_ESCAPE};
    c.inputEvent = "ToggleMouse";

    InputManager::GetSingleton()->m_InputScheme.bools.push_back(a);
    InputManager::GetSingleton()->m_InputScheme.bools.push_back(b);
    InputManager::GetSingleton()->m_InputScheme.bools.push_back(c);

    InputManager::GetSingleton()->m_InputMap.insert(
        eastl::pair<eastl::string, InputScheme>("DefaultInputScheme", InputManager::GetSingleton()->m_InputScheme));

    InputManager::LoadSchemes(InputManager::GetSingleton()->m_InputMap);
    InputManager::PushScheme("DefaultInputScheme");
    InputManager::SetEnabled(true);

    // Set the mouse mode to use in the sample
    InitMouseMode(MM_RELATIVE);
}

void InputManagerTest::Stop()
{
    // This step is executed when application is closing. No more frames will be rendered after this method is invoked.
    App::Stop();
}

void InputManagerTest::SubscribeToEvents()
{
    // Subscribe to Update event for setting the character controls before physics simulation
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(InputManagerTest, HandleUpdate));

    // Subscribe to PostUpdate event for updating the camera position after physics simulation
    SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(InputManagerTest, HandlePostUpdate));

    SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(InputManagerTest, HandlePostRenderUpdate));

    // Unsubscribe the SceneUpdate event from base class as the camera node is being controlled in HandlePostUpdate() in
    // this sample
    UnsubscribeFromEvent(E_SCENEUPDATE);
}

void InputManagerTest::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;

    if (InputManager::IsDown("LongPress"))
    {
        URHO3D_LOGINFO("Long Press");
    }

    if (InputManager::IsPressed("OnePress"))
    {
        URHO3D_LOGINFO("One Press");
    }

    if (InputManager::MouseButtonDown(MouseButton::MOUSEB_MIDDLE))
    {
        URHO3D_LOGINFO("Mouse Location {} {}", InputManager::GetMousePosition().x_,
                       InputManager::GetMousePosition().y_);
        URHO3D_LOGINFO("Mouse Wheel Delta {}", InputManager::GetMouseMoveWheel());
    }

    if (InputManager::IsPressed("ToggleMouse"))
    {
        InputManager::SetMouseVisible(!InputManager::IsMouseVisible());
        URHO3D_LOGINFO("Mouse Visibility: {}", InputManager::IsMouseVisible());
    }
}

void InputManagerTest::HandlePostUpdate(StringHash eventType, VariantMap& eventData) { return; }

void InputManagerTest::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData) { return; }
