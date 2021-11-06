#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/UI/UI.h>
#if URHO3D_SYSTEMUI
#include <Urho3D/SystemUI/DebugHud.h>
#endif
#if URHO3D_RMLUI
#include <Urho3D/RmlUI/RmlUI.h>
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

#ifndef __EMSCRIPTEN__
static BaseApplication* app = nullptr;
#endif

BaseApplication::BaseApplication(Context* context)
    : Application(context)
{
    engineParameters_[EP_VSYNC] = true;
    engineParameters_[EP_WINDOW_MAXIMIZE] = true;
    engineParameters_[EP_FULL_SCREEN] = false;
    engineParameters_[EP_WINDOW_HEIGHT] = 720;
    engineParameters_[EP_WINDOW_WIDTH] = 1280;
    engineParameters_[EP_RESOURCE_PATHS] = "C:\\THEO\\Projects\\CharacterInteraction";
    engineParameters_[EP_RESOURCE_PREFIX_PATHS] = "CoreData;Resources;Cache";
    engineParameters_[EP_LOG_NAME] = "STAR.log";

    GetSubsystem<ResourceCache>()->AddResourceDir("C:\\THEO\\Projects\\CharacterInteraction\\CoreData");
    GetSubsystem<ResourceCache>()->AddResourceDir("C:\\THEO\\Projects\\CharacterInteraction\\Resources");
    GetSubsystem<ResourceCache>()->AddResourceDir("C:\\THEO\\Projects\\CharacterInteraction\\Cache");

#ifndef __EMSCRIPTEN__
    app = this;
    signal(SIGINT,
           [](int signum)
           {
               URHO3D_LOGINFOF("Signal '{%d}' received, exiting app...", signum);
               if (app)
               {
                   app->Exit();
                   app = nullptr;
               }
           });
#endif

    ConfigFile::RegisterObject(context);
    ConfigManager::RegisterObject(context);

    context_->RegisterFactory<ControllerInput>();
    context_->RegisterFactory<Notifications>();
    context_->RegisterFactory<Achievements>();
    SingleAchievement::RegisterObject(context_);
    State::RegisterObject(context_);

#ifdef PACKAGE_MANAGER
    PackageManager::RegisterObject(context_);
#endif

    LevelManager::RegisterObject(context_);

#ifdef NAKAMA_SUPPORT
    NakamaManager::RegisterObject(context_);
#endif

#if defined(__EMSCRIPTEN__)
    appContext = context;
#endif

    context_->RegisterFactory<WindowManager>();
    context_->RegisterFactory<AudioManager>();
    context_->RegisterFactory<ConsoleHandler>();
    //SceneManager::RegisterObject(context_); // Figure out where else this is being registered
    //context_->RegisterFactory<Generator>();

    BehaviourTree::RegisterFactory(context_);

#ifdef __ANDROID__
    configurationFile_ = GetSubsystem<FileSystem>()->GetUserDocumentsDir() + DOCUMENTS_DIR + "/config.cfg";
#else

#ifdef __EMSCRIPTEN__
    configurationFile_ = GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Config/config.cfg";
#else
    configurationFile_ = "Config/config.cfg"; //"Data/Config/config.cfg";
#endif
#endif

    ConfigManager* configManager = new ConfigManager(context_, configurationFile_);
    context_->RegisterSubsystem(configManager);
    context_->RegisterSubsystem(new State(context_));

#ifdef PACKAGE_MANAGER
    context_->RegisterSubsystem(new PackageManager(context_));
#endif

    context_->RegisterSubsystem(new SceneManager(context_));

    context_->RegisterSubsystem(new ServiceCmd(context_));
#ifdef NAKAMA_SUPPORT
    context_->RegisterSubsystem(new NakamaManager(context_));
#endif

#ifdef __ANDROID__
    String directory = GetSubsystem<FileSystem>()->GetUserDocumentsDir() + DOCUMENTS_DIR;
    if (!GetSubsystem<FileSystem>()->DirExists(directory))
    {
        GetSubsystem<FileSystem>()->CreateDir(directory);
    }
#endif
}

void BaseApplication::Setup()
{
    context_->RegisterSubsystem(new ConsoleHandler(context_));

    LoadINIConfig(configurationFile_);
}

void BaseApplication::Start()
{
#ifdef NAKAMA_SUPPORT
    GetSubsystem<NakamaManager>()->Init();
#endif

#ifdef PACKAGE_MANAGER
    GetSubsystem<PackageManager>()->Init();
#endif

    GetSubsystem<ServiceCmd>()->Init();
    UI* ui = GetSubsystem<UI>();
    auto cache = GetSubsystem<ResourceCache>();

#ifdef __ANDROID__
    ui->SetScale(1.8);
#else
    ui->SetScale(GetSubsystem<ConfigManager>()->GetFloat("engine", "UIScale", 1.0));
#endif

#ifdef __APPLE__
    ui->SetScale(2.0);
    GetSubsystem<ConfigManager>()->Set("engine", "HighDPI", true);
#endif

    if (!GetSubsystem<Engine>()->IsHeadless())
    {
        GetSubsystem<ConsoleHandler>()->Create();
        DebugHud* debugHud = GetSubsystem<Engine>()->CreateDebugHud();
    }

    cache->SetAutoReloadResources(true);
    ui->GetRoot()->SetDefaultStyle(cache->GetResource<XMLFile>("UI/DefaultStyle.xml"));

    SubscribeToEvents();

    GetSubsystem<FileSystem>()->SetExecuteConsoleCommands(false);


#if defined(URHO3D_LUA) || defined(URHO3D_ANGELSCRIPT)
    context_->RegisterSubsystem(new ModLoader(context_));
#endif

    context_->RegisterSubsystem(new AudioManager(context_));

    // Allow multiple music tracks to play at the same time
    context_->GetSubsystem<AudioManager>()->AllowMultipleMusicTracks(true);
    // Allow multiple ambient tracks to play at the same time
    context_->GetSubsystem<AudioManager>()->AllowMultipleAmbientTracks(true);

    using namespace AudioDefs;
    context_->GetSubsystem<AudioManager>()->RegisterAudio(SOUND_EFFECTS::HIT, "Sounds/PlayerFistHit.wav");
    context_->GetSubsystem<AudioManager>()->RegisterAudio(SOUND_EFFECTS::THROW, "Sounds/NutThrow.wav");
    context_->GetSubsystem<AudioManager>()->RegisterAudio(SOUND_EFFECTS::BUTTON_CLICK, "Sounds/click.wav");
    context_->GetSubsystem<AudioManager>()->RegisterAudio(SOUND_EFFECTS::ACHIEVEMENT, "Sounds/achievement.wav");
    context_->GetSubsystem<AudioManager>()->RegisterAudio(SOUND_EFFECTS::PLACE_BLOCK, "Sounds/place_block.wav");

    context_->GetSubsystem<AudioManager>()->RegisterAudio(MUSIC::GAME, "Sounds/music.wav");
    context_->GetSubsystem<AudioManager>()->RegisterAudio(MUSIC::MENU, "Sounds/menu.wav");

    context_->GetSubsystem<AudioManager>()->RegisterAudio(AMBIENT_SOUNDS::LEVEL, "Sounds/ambient.wav");

    auto controllerInput = new ControllerInput(context_);
    context_->RegisterSubsystem(controllerInput);
    controllerInput->LoadConfig();

    context_->RegisterSubsystem(new Notifications(context_));

    RegisterConsoleCommands();

    ApplyGraphicsSettings();

    // Initialize the first level from the config file
    VariantMap& eventData = GetEventDataMap();
    if (GetSubsystem<ConfigManager>()->GetBool("server", "Dedicated", false))
    {
        eventData["Name"] = "Loading";
        eventData["StartServer"] = true;
        eventData["Map"] = GetSubsystem<ConfigManager>()->GetString("server", "Map", "Scenes/Flat.xml");
    }
    else
    {
        eventData["Name"] = GetSubsystem<ConfigManager>()->GetString("game", "FirstLevel", "Splash");
    }
    SendEvent(E_SET_LEVEL, eventData);

    LoadTranslationFiles();

    SendEvent("GameStarted");
}

void BaseApplication::Stop() {}

void BaseApplication::LoadConfig(eastl::string filename, eastl::string prefix, bool isMain)
{
    URHO3D_LOGINFO("Loading config file '" + filename + "' with '" + prefix + "' prefix");
    JSONFile json(context_);
    json.LoadFile(GetSubsystem<FileSystem>()->GetProgramDir() + filename);
    JSONValue& content = json.GetRoot();
    if (content.IsObject())
    {
        for (auto& it : content)
        {

            // If it's the main config file, we should only then register this
            // config parameter key for saving
            if (isMain)
            {
                globalSettings_[StringHash((it).first)] = (it).first;
            }
            if ((it).second.IsBool())
            {
                engine_->SetGlobalVar(prefix + (it).first, (it).second.GetBool());
            }
            if ((it).second.IsString())
            {
                engine_->SetGlobalVar(prefix + (it).first, (it).second.GetString());
            }
            if ((it).second.IsNumber())
            {
                if ((it).second.GetNumberType() == JSONNT_FLOAT_DOUBLE)
                {
                    engine_->SetGlobalVar(prefix + (it).first, (it).second.GetFloat());
                }
                if ((it).second.GetNumberType() == JSONNT_INT)
                {
                    engine_->SetGlobalVar(prefix + (it).first, (it).second.GetInt());
                }
            }
        }
    }
    else
    {
        URHO3D_LOGERROR("Config file " + filename + " format is not correct!");
    }
}

void BaseApplication::HandleLoadConfig(StringHash eventType, VariantMap& eventData)
{
    eastl::string filename = eventData["Filepath"].GetString();
    eastl::string prefix = eventData["Prefix"].GetString();
    if (!filename.empty())
    {
        LoadConfig(filename, prefix);
    }
}

void BaseApplication::RegisterConsoleCommands()
{
    VariantMap& data = GetEventDataMap();
    data["ConsoleCommandName"] = "exit";
    data["ConsoleCommandEvent"] = "HandleExit";
    data["ConsoleCommandDescription"] = "Exits game";
    SendEvent("ConsoleCommandAdd", data);

    SubscribeToEvent("HandleExit", URHO3D_HANDLER(BaseApplication, HandleExit));

    SendEvent(E_CONSOLE_COMMAND_ADD, ConsoleCommandAdd::P_NAME, "debugger", ConsoleCommandAdd::P_EVENT, "#debugger",
              ConsoleCommandAdd::P_DESCRIPTION, "Show debug");
    SubscribeToEvent("#debugger",
                     [&](StringHash eventType, VariantMap& eventData)
                     {
                         if (GetSubsystem<DebugHud>())
                         {
                             GetSubsystem<DebugHud>()->Toggle(DEBUGHUD_SHOW_STATS);
                         }
                     });

    SendEvent(E_CONSOLE_COMMAND_ADD, ConsoleCommandAdd::P_NAME, "mouse_visible", ConsoleCommandAdd::P_EVENT,
              "#mouse_visible", ConsoleCommandAdd::P_DESCRIPTION, "Toggle mouse visible");
    SubscribeToEvent("#mouse_visible",
                     [&](StringHash eventType, VariantMap& eventData)
                     {
                         Input* input = GetSubsystem<Input>();
                         if (input->IsMouseVisible())
                         {
                             input->SetMouseVisible(!input->IsMouseVisible());
                         }
                     });
}

void BaseApplication::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    using namespace KeyDown;

    //int key = eventData[P_KEY].GetInt();
}

void BaseApplication::HandleKeyUp(StringHash eventType, VariantMap& eventData) {}

void BaseApplication::HandleExit(StringHash eventType, VariantMap& eventData) { GetSubsystem<Engine>()->Exit(); }

void BaseApplication::Exit() { GetSubsystem<Engine>()->Exit(); }

void BaseApplication::SubscribeToEvents()
{
    SubscribeToEvent(E_ADD_CONFIG, URHO3D_HANDLER(BaseApplication, HandleAddConfig));
    SubscribeToEvent(E_LOAD_CONFIG, URHO3D_HANDLER(BaseApplication, HandleLoadConfig));

    SubscribeToEvent(E_MAPPED_CONTROL_RELEASED,
                     [&](StringHash eventType, VariantMap& eventData)
                     {
                         using namespace MappedControlReleased;
                         int action = eventData[P_ACTION].GetInt();
                         if (action == CTRL_SCREENSHOT)
                         {
                             Graphics* graphics = GetSubsystem<Graphics>();
                             Image screenshot(context_);
                             graphics->TakeScreenShot(screenshot);
                             // Here we save in the Data folder with date and time appended
                             screenshot.SavePNG(
                                 GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Screenshot_" +
                                 Time::GetTimeStamp().replaced(':', '_').replaced('.', '_').replaced(' ', '_') +
                                 ".png");

                             SendEvent("ScreenshotTaken");
                         }
                     });

    // Subscribe key down event
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(BaseApplication, HandleKeyDown));
    // Subscribe key up event
    SubscribeToEvent(E_KEYUP, URHO3D_HANDLER(BaseApplication, HandleKeyUp));
}

void BaseApplication::HandleAddConfig(StringHash eventType, VariantMap& eventData)
{
    eastl::string paramName = eventData["Name"].GetString();
    if (!paramName.empty())
    {
        globalSettings_[paramName] = paramName;
    }
}

void BaseApplication::LoadINIConfig(eastl::string filename)
{
    bool loaded = GetSubsystem<ConfigManager>()->Load(filename, true);
    URHO3D_LOGINFO("{}", loaded);

    SetEngineParameter(EP_MONITOR, GetSubsystem<ConfigManager>()->GetInt("video", "Monitor", 0));
    int windowMode = GetSubsystem<ConfigManager>()->GetInt("video", "WindowMode", 0);
    SetEngineParameter(EP_FULL_SCREEN, windowMode == 2);
    SetEngineParameter(EP_BORDERLESS, windowMode == 1);

    // Dedicated server - headless mode
    SetEngineParameter(EP_HEADLESS, GetSubsystem<ConfigManager>()->GetBool("server", "Dedicated", false));

    SetEngineParameter(EP_WINDOW_WIDTH, GetSubsystem<ConfigManager>()->GetInt("video", "Width", 1280));
    SetEngineParameter(EP_WINDOW_HEIGHT, GetSubsystem<ConfigManager>()->GetInt("video", "Height", 720));
    SetEngineParameter(EP_VSYNC, GetSubsystem<ConfigManager>()->GetBool("video", "VSync", true));
    SetEngineParameter(EP_REFRESH_RATE, GetSubsystem<ConfigManager>()->GetInt("video", "RefreshRate", 60));
#ifdef __EMSCRIPTEN
    SetEngineParameter(EP_WINDOW_RESIZABLE, true);
#else
    SetEngineParameter(EP_WINDOW_RESIZABLE, GetSubsystem<ConfigManager>()->GetBool("video", "ResizableWindow", false));
    SetEngineParameter(EP_BORDERLESS, false);
#endif

    // Engine settings
    GetSubsystem<Engine>()->SetMaxFps(GetSubsystem<ConfigManager>()->GetInt("engine", "FPSLimit", 60));
    SetEngineParameter(EP_HIGH_DPI, GetSubsystem<ConfigManager>()->GetBool("engine", "HighDPI", false));
    SetEngineParameter(EP_WINDOW_TITLE, "STAR");
    SetEngineParameter(EP_WINDOW_ICON, "Textures/AppIcon.png");

    // Logs
    SetEngineParameter(EP_LOG_NAME,
                       GetSubsystem<ConfigManager>()->GetString("engine", "LogName", "ProjectTemplate.log"));
    SetEngineParameter(EP_LOG_LEVEL, GetSubsystem<ConfigManager>()->GetInt("engine", "LogLevel", LOG_INFO));
    SetEngineParameter(EP_LOG_QUIET, GetSubsystem<ConfigManager>()->GetBool("engine", "LogQuiet", false));
    SetEngineParameter(EP_RESOURCE_PATHS,
                       GetSubsystem<ConfigManager>()->GetString("engine", "ResourcePaths", "Data;CoreData"));
    SetEngineParameter(EP_FLUSH_GPU, GetSubsystem<ConfigManager>()->GetBool("engine", "FlushGPU", true));
    SetEngineParameter(EP_WORKER_THREADS, GetSubsystem<ConfigManager>()->GetBool("engine", "WorkerThreads ", true));

    SetEngineParameter("Master", GetSubsystem<ConfigManager>()->GetFloat("audio", "Master", 1.0));
    SetEngineParameter("Effect", GetSubsystem<ConfigManager>()->GetFloat("audio", "Effect", 1.0));
    SetEngineParameter("Ambient", GetSubsystem<ConfigManager>()->GetFloat("audio", "Ambient", 1.0));
    SetEngineParameter("Voice", GetSubsystem<ConfigManager>()->GetFloat("audio", "Voice", 1.0));
    SetEngineParameter("Music", GetSubsystem<ConfigManager>()->GetFloat("audio", "Music", 1.0));

    Audio* audio = GetSubsystem<Audio>();

    //audio->SetMasterGain(SOUND_MASTER, engine_->GetGlobalVar("Master").GetFloat());
    //audio->SetMasterGain(SOUND_EFFECT, engine_->GetGlobalVar("Effect").GetFloat());
    //audio->SetMasterGain(SOUND_AMBIENT, engine_->GetGlobalVar("Ambient").GetFloat());
    //audio->SetMasterGain(SOUND_VOICE, engine_->GetGlobalVar("Voice").GetFloat());
    //audio->SetMasterGain(SOUND_MUSIC, engine_->GetGlobalVar("Music").GetFloat());

    // GetSubsystem<Log>()->SetTimeStamp(GetSubsystem<ConfigManager>()->GetBool("engine", "LogTimestamp", true));
}

void BaseApplication::ApplyGraphicsSettings()
{
    auto* renderer = GetSubsystem<Renderer>();
    if (renderer)
    {
        renderer->SetTextureQuality((Urho3D::MaterialQuality)GetSubsystem<ConfigManager>()->GetInt(
            "graphics", "TextureQuality", MaterialQuality::QUALITY_HIGH));
        renderer->SetMaterialQuality((Urho3D::MaterialQuality)GetSubsystem<ConfigManager>()->GetInt(
            "graphics", "MaterialQuality", MaterialQuality::QUALITY_MAX));
        renderer->SetDrawShadows(GetSubsystem<ConfigManager>()->GetBool("graphics", "DrawShadows", true));
        renderer->SetShadowMapSize(GetSubsystem<ConfigManager>()->GetInt("graphics", "ShadowMapSize", 512));
        renderer->SetShadowQuality((ShadowQuality)GetSubsystem<ConfigManager>()->GetInt(
            "graphics", "ShadowQuality", ShadowQuality::SHADOWQUALITY_PCF_16BIT));
        renderer->SetMaxOccluderTriangles(
            GetSubsystem<ConfigManager>()->GetInt("graphics", "MaxOccluderTriangles", 5000));
        renderer->SetDynamicInstancing(GetSubsystem<ConfigManager>()->GetBool("graphics", "DynamicInstancing", true));
        renderer->SetSpecularLighting(GetSubsystem<ConfigManager>()->GetBool("graphics", "SpecularLighting", true));
        renderer->SetHDRRendering(GetSubsystem<ConfigManager>()->GetBool("graphics", "HDRRendering", false));
    }
}

void BaseApplication::SetEngineParameter(eastl::string parameter, Variant value)
{
    engineParameters_[parameter] = value;
    engine_->SetGlobalVar(parameter, value);
    globalSettings_[parameter] = parameter;
}

void BaseApplication::LoadTranslationFiles()
{
    eastl::vector<eastl::string> result;
    auto* localization = GetSubsystem<Localization>();

    // Get all translation files in the Data/Translations folder
    GetSubsystem<FileSystem>()->ScanDir(
        result, GetSubsystem<FileSystem>()->GetProgramDir() + eastl::string("Data/Translations"),
        eastl::string("*.json"), SCAN_FILES, true);

    // Get all the translations from packages
    auto packageFiles = GetSubsystem<ResourceCache>()->GetPackageFiles();
    for (auto it = packageFiles.begin(); it != packageFiles.end(); ++it)
    {
        auto files = (*it)->GetEntryNames();
        for (auto it2 = files.begin(); it2 != files.end(); ++it2)
        {
            if ((*it2).starts_with("Translations/") && (*it2).ends_with(".json") && (*it2).split('/').size() == 2)
            {
                result.push_back((*it2).split('/').at(1));
            }
        }
    }

    for (auto it = result.begin(); it != result.end(); ++it)
    {
        eastl::string file = (*it);

        eastl::string filepath = "Translations/" + file;
        // Filename is handled as a language
        file.replace(".json", "", false);

        auto jsonFile = GetSubsystem<ResourceCache>()->GetResource<JSONFile>(filepath);
        if (jsonFile)
        {
            // Load the actual file in the system
            localization->LoadSingleLanguageJSON(jsonFile->GetRoot(), file);
            URHO3D_LOGINFO("Loading translation file '{}' to '{}' language", filepath, file);
        }
        else
        {
            URHO3D_LOGERROR("Translation file '{}' not found!", filepath);
        }
    }

    // Finally set the application language
    localization->SetLanguage(GetSubsystem<ConfigManager>()->GetString("game", "Language", "EN"));
}
