
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/StringUtils.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/IO/Archive.h>
#include <Urho3D/IO/ArchiveSerialization.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Input/InputManager.h>
#include <Urho3D/Resource/JSONArchive.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/ResourceEvents.h>

#include "STAR.h"

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

    // Load Input Map
    InputManager::LoadInputMapFromFile("Config/InputMap.json");
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
        if (InputManager::IsMouseVisible())
        {
            InputManager::SetMouseVisible(false);
        }
        else
        {
            InputManager::SetMouseVisible(true);
        }
        URHO3D_LOGINFO("Mouse Visibility: {}", InputManager::IsMouseVisible());
    }
}

void InputManagerTest::HandlePostUpdate(StringHash eventType, VariantMap& eventData) { return; }

void InputManagerTest::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData) { return; }
