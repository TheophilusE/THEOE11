
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/UI/UI.h>
#if URHO3D_SYSTEMUI
#include <Urho3D/SystemUI/DebugHud.h>
#endif
#include "AndroidEvents/ServiceCmd.h"
#include "Audio/AudioManager.h"
#include "Audio/AudioManagerDefs.h"
#include "BaseApplication.h"
#include "BehaviourTree/BehaviourTree.h"
#include "Config/ConfigFile.h"
#include "Console/ConsoleHandler.h"
#include "CustomEvents.h"
#include "Generator/Generator.h"
#include "Globals/Settings.h"
#include "Input/ControllerInput.h"
#include "SceneManager.h"
#include "State/State.h"
#include "signal.h"
#include <Urho3D/Audio/Audio.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/IO/PackageFile.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Resource/Localization.h>

#ifdef PACKAGE_MANAGER
#include "PackageManager/PackageManager.h"
#endif

#include "Console/ConsoleHandlerEvents.h"
#include "Input/ControlDefines.h"
#include "Input/ControllerEvents.h"
#include "LevelManagerEvents.h"

#ifdef NAKAMA_SUPPORT
#include "Nakama/NakamaManager.h"
#endif

#if defined(__EMSCRIPTEN__)
#include <Urho3D/Graphics/GraphicsEvents.h>
#include <Urho3D/Input/InputEvents.h>
#include <emscripten/bind.h>
#include <emscripten/emscripten.h>
static const Context* appContext;
static bool mouseVisible;
static unsigned int mouseMode;
#endif

using namespace ConsoleHandlerEvents;
using namespace LevelManagerEvents;
using namespace ControllerEvents;
using namespace CustomEvents;

#include "STAR.h"

Star::Star(Context* context)
    : BaseApplication(context)
{
    Setup();
}

void Star::Setup()
{
    BaseApplication::Setup();

}

void Star::Start()
{
    BaseApplication::Start();

    // Subscribe to necessary events
    SubscribeToEvents();
}

void Star::Stop() { BaseApplication::Stop(); }

void Star::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    
}

void Star::SubscribeToEvents()
{
    // Subscribe to Update event for setting the character controls before physics simulation
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Star, HandleUpdate));
}
