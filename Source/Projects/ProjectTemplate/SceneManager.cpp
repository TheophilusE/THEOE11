#include <Urho3D/Core/Context.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/IO/IOEvents.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/SceneEvents.h>
#if URHO3D_SYSTEMUI
#include <Urho3D/SystemUI/DebugHud.h>
#endif

#include "Console/ConsoleHandlerEvents.h"
#include "LevelManagerEvents.h"
#include "SceneManager.h"
#include "SceneManagerEvents.h"
#include <Urho3D/Core/CoreEvents.h>

using namespace Urho3D;
using namespace ConsoleHandlerEvents;
using namespace SceneManagerEvents;
using namespace LevelManagerEvents;

/**
 * Wait this many MS before marking loading step as completed if no ACK request was received
 */
const int LOADING_STEP_ACK_MAX_TIME = 2000;            // Max wait time in MS for ACK message for loading step
const int LOADING_STEP_MAX_EXECUTION_TIME = 10 * 1000; // Max loading step execution time in MS, 0 - infinite
const float PROGRESS_SPEED =
    1.0f; // how fast should the progress bar increase each second, e.g. 1 would load 0 to 100% in 1 second

SceneManager::SceneManager(Context* context)
    : Object(context)
{
    SubscribeToEvent(E_REGISTER_LOADING_STEP, URHO3D_HANDLER(SceneManager, HandleRegisterLoadingStep));
    SubscribeToEvent(E_ADD_MAP, URHO3D_HANDLER(SceneManager, HandleAddMap));
    SubscribeToEvent(E_LOADING_STEP_CRITICAL_FAIL,
                     [&](StringHash eventType, VariantMap& eventData)
                     {
                         UnsubscribeFromEvent(E_UPDATE);
                         using namespace LoadingStepCriticalFail;
                         VariantMap& data = GetEventDataMap();
                         data["Name"] = "MainMenu";
                         data["Type"] = "error";
                         data["Message"] = eventData[P_DESCRIPTION];
                         SendEvent(E_SET_LEVEL, data);
                     });

    LoadDefaultMaps();
}

SceneManager::~SceneManager() {}

void SceneManager::RegisterObject(Context* context) { context->RegisterFactory<SceneManager>(); }

void SceneManager::LoadScene(const eastl::string& filename)
{
    ResetProgress();
    activeScene_.Reset();
    activeScene_ = new Scene(context_);
    activeScene_->SetAsyncLoadingMs(1);
    auto xmlFile = GetSubsystem<ResourceCache>()->GetFile(filename);
    activeScene_->LoadAsyncXML(xmlFile);
    loadingStatus_ = "Loading scene";

    if (GetSubsystem<DebugHud>())
    {
        GetSubsystem<DebugHud>()->SetAppStats("Scene manager map", filename);
    }
    URHO3D_LOGINFO("Scene manager loading scene: {}", filename);

    SubscribeToEvent(activeScene_, E_ASYNCLOADPROGRESS, URHO3D_HANDLER(SceneManager, HandleAsyncSceneLoadingProgress));
    SubscribeToEvent(activeScene_, E_ASYNCEXECFINISHED, URHO3D_HANDLER(SceneManager, HandleAsyncSceneLoadingFinished));
    SubscribeToEvent(activeScene_, E_ASYNCLOADFINISHED, URHO3D_HANDLER(SceneManager, HandleAsyncSceneLoadingFinished));

    SendEvent(E_CONSOLE_COMMAND_ADD, ConsoleCommandAdd::P_NAME, "remove_local_nodes", ConsoleCommandAdd::P_EVENT,
              "#remove_local_nodes", ConsoleCommandAdd::P_DESCRIPTION, "Remove all local nodes");
    SubscribeToEvent("#remove_local_nodes",
                     [&](StringHash eventType, VariantMap& eventData)
                     {
                         if (activeScene_)
                         {
                             activeScene_->Clear(false, true);
                         }
                     });

    SendEvent(E_CONSOLE_COMMAND_ADD, ConsoleCommandAdd::P_NAME, "remove_replicated_nodes", ConsoleCommandAdd::P_EVENT,
              "#remove_replicated_nodes", ConsoleCommandAdd::P_DESCRIPTION, "Remove all replicated nodes");
    SubscribeToEvent("#remove_replicated_nodes",
                     [&](StringHash eventType, VariantMap& eventData)
                     {
                         if (activeScene_)
                         {
                             activeScene_->Clear(true, false);
                         }
                     });
}

void SceneManager::HandleAsyncSceneLoadingProgress(StringHash eventType, VariantMap& eventData)
{
    using namespace AsyncLoadProgress;
    float progress = eventData[P_PROGRESS].GetFloat();
    int nodesLoaded = eventData[P_LOADEDNODES].GetInt();
    int totalNodes = eventData[P_TOTALNODES].GetInt();
    int resourcesLoaded = eventData[P_LOADEDRESOURCES].GetInt();
    int totalResources = eventData[P_TOTALRESOURCES].GetInt();

    URHO3D_LOGINFOF("Loading progress {%f} {%i}/{%i} {%i}/{%i}", progress, nodesLoaded, totalNodes, resourcesLoaded,
                    totalResources);
}

void SceneManager::HandleAsyncSceneLoadingFinished(StringHash eventType, VariantMap& eventData)
{
    using namespace AsyncLoadFinished;

    activeScene_->SetUpdateEnabled(false);
    currentMap_ = GetMap(activeScene_->GetFileName());

    UnsubscribeFromEvent(E_ASYNCLOADPROGRESS);
    UnsubscribeFromEvent(E_ASYNCLOADFINISHED);

    SubscribeToEvent(E_ACK_LOADING_STEP, URHO3D_HANDLER(SceneManager, HandleLoadingStepAck));
    SubscribeToEvent(E_LOADING_STEP_PROGRESS, URHO3D_HANDLER(SceneManager, HandleLoadingStepProgress));
    SubscribeToEvent(E_LOADING_STEP_FINISHED, URHO3D_HANDLER(SceneManager, HandleLoadingStepFinished));
    SubscribeToEvent(E_LOADING_STEP_SKIP, URHO3D_HANDLER(SceneManager, HandleSkipLoadingStep));

    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(SceneManager, HandleUpdate));
    URHO3D_LOGINFO("Scene loaded: " + activeScene_->GetFileName());
}

void SceneManager::CleanupLoadingSteps()
{
    for (auto& it : loadingSteps_)
    {
        if (it.second.autoRemove)
        {
            URHO3D_LOGINFOF("Auto removing loading step {%s}", it.second.name.c_str());
            loadingSteps_.erase(it.first);
        }
    }
}

void SceneManager::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;
    progress_ += eventData[P_TIMESTEP].GetFloat() * PROGRESS_SPEED;
    if (progress_ > targetProgress_)
    {
        progress_ = targetProgress_;
    }

    float completed = 1;
    targetProgress_ = (float)completed / ((float)loadingSteps_.size() + 1.0f);
    for (auto it = loadingSteps_.begin(); it != loadingSteps_.end(); ++it)
    {
        if ((*it).second.finished)
        {
            completed++;
        }
        if ((*it).second.ackSent && !(*it).second.ack &&
            (*it).second.ackTimer.GetMSec(false) > LOADING_STEP_ACK_MAX_TIME)
        {
            (*it).second.finished = true;
            (*it).second.ack = true;
        }
        targetProgress_ = (float)completed / ((float)loadingSteps_.size() + 1.0f);

        if (!(*it).second.map.empty() && (*it).second.map != activeScene_->GetFileName())
        {
            (*it).second.finished = true;
            continue;
        }

        if (CanLoadingStepRun((*it).second))
        {

            // TODO: implement fix for web builds as the loading steps might take longer to execute
            // due to the inactive browsers tabs where game is running in the background
            // Handle loading steps which take too much time to execute
            if ((*it).second.ackSent &&
                (*it).second.ackTimer.GetMSec(false) > LOADING_STEP_MAX_EXECUTION_TIME + LOADING_STEP_ACK_MAX_TIME)
            {
                (*it).second.finished = true;
                (*it).second.failed = true;
                URHO3D_LOGERROR("Loading step '{}' failed, took too long to execute!", (*it).second.name);

                using namespace LoadingStepTimedOut;
                VariantMap& data = GetEventDataMap();
                data[P_EVENT] = (*it).second.event;
                SendEvent(E_LOADING_STEP_TIMED_OUT, data);
                URHO3D_LOGINFO("Loading step skipped, no ACK retrieved for {}", (*it).second.name);
                return;

                // Note the the tasks could still succeed in the background, but the loading screen will move further
                // without waiting it to finish
                return;
            }
            if (!(*it).second.ackSent)
            {

                if (loadingStatus_ != (*it).second.name)
                {
                    loadingStatus_ = (*it).second.name;
                    // Delay loading step execution till the next frame
                    // to allow the status to be updated
                    using namespace LoadingStatusUpdate;
                    VariantMap data;
                    data[P_NAME] = (*it).second.name;
                    SendEvent(E_LOADING_STATUS_UPDATE, data);
                    return;
                }

                VariantMap data;
                data["Map"] = activeScene_->GetFileName();
                // Send out event to start this loading step
                SendEvent((*it).second.event, data);

                // We register that start event was sent out, loading step must send back ACK message
                // to let us know that the loading step was started, otherwise it will be automatically
                // marked as a finished job, to avoid app inifite loading
                (*it).second.ackSent = true;
                (*it).second.ackTimer.Reset();
            }
            completed += (*it).second.progress;
            targetProgress_ = (float)completed / ((float)loadingSteps_.size() + 1.0f);
            return;
        }
    }

    if (progress_ >= 1.0f)
    {
        progress_ = 1.0f;
        UnsubscribeFromEvent(E_UPDATE);

        // Re-enable active scene
        activeScene_->SetUpdateEnabled(true);

        UnsubscribeFromEvent(E_ACK_LOADING_STEP);
        UnsubscribeFromEvent(E_LOADING_STEP_PROGRESS);
        UnsubscribeFromEvent(E_LOADING_STEP_FINISHED);
        UnsubscribeFromEvent(E_LOADING_STEP_SKIP);

        CleanupLoadingSteps();
    }
}

void SceneManager::ResetProgress()
{
    progress_ = 0.0f;
    targetProgress_ = 0.0f;

    for (auto it = loadingSteps_.begin(); it != loadingSteps_.end(); ++it)
    {
        (*it).second.finished = false;
        (*it).second.ack = false;
        (*it).second.ackSent = false;
        (*it).second.progress = 0.0f;
    }
}

void SceneManager::HandleRegisterLoadingStep(StringHash eventType, VariantMap& eventData)
{
    using namespace RegisterLoadingStep;
    LoadingStep step;
    step.name = eventData[P_NAME].GetString();
    step.event = eventData[P_EVENT].GetString();
    step.map = eventData[P_MAP].GetString();
    step.ackSent = false;
    step.finished = false;
    step.failed = false;
    step.progress = 0;
    step.autoRemove = false;
    step.dependsOn = eventData[P_DEPENDS_ON].GetStringVector();

    if (eventData.contains(P_REMOVE_ON_FINISH) && eventData[P_REMOVE_ON_FINISH].GetBool())
    {
        step.autoRemove = true;
    }
    if (step.name.empty() || step.event.empty())
    {
        URHO3D_LOGERROR("Unable to register loading step " + step.name + ":" + step.event);
        return;
    }

    URHO3D_LOGINFO("Registering new loading step: " + step.name + "; " + step.event);
    loadingSteps_[step.event] = step;

    if (GetSubsystem<DebugHud>())
    {
        GetSubsystem<DebugHud>()->SetAppStats("Loading steps", loadingSteps_.size());
    }
}

void SceneManager::HandleLoadingStepAck(StringHash eventType, VariantMap& eventData)
{
    //
    using namespace AckLoadingStep;
    eastl::string name = eventData[P_EVENT].GetString();

    loadingSteps_[name].ack = true;
    loadingSteps_[name].loadTime.Reset();
    URHO3D_LOGINFO("Loading step '{}' acknowledged", name);
}

void SceneManager::HandleLoadingStepProgress(StringHash eventType, VariantMap& eventData)
{
    using namespace LoadingStepProgress;
    eastl::string event = eventData[P_EVENT].GetString();
    float progress = eventData[P_PROGRESS].GetFloat();
    progress = Clamp(progress, 0.0f, 1.0f);
    loadingSteps_[event].progress = progress;

    URHO3D_LOGINFO("Loading step progress update '{}' : {}", event, progress);
}

void SceneManager::HandleLoadingStepFinished(StringHash eventType, VariantMap& eventData)
{
    using namespace LoadingStepFinished;
    eastl::string event = eventData[P_EVENT].GetString();

    loadingSteps_[event].finished = true;

    URHO3D_LOGINFO("Loading step {} event finished", event);
}

void SceneManager::HandleSkipLoadingStep(StringHash eventType, VariantMap& eventData)
{
    using namespace LoadingStepSkip;
    eastl::string event = eventData[P_EVENT].GetString();
    if (loadingSteps_.contains(event))
    {
        loadingSteps_[event].finished = true;
    }
}

MapInfo* SceneManager::GetMap(const eastl::string& filename)
{
    for (auto it = availableMaps_.begin(); it != availableMaps_.end(); ++it)
    {
        if ((*it).map == filename)
        {
            return &(*it);
        }
    }
    return nullptr;
}

void SceneManager::HandleAddMap(StringHash eventType, VariantMap& eventData)
{
    using namespace AddMap;
    eastl::string map = eventData[P_MAP].GetString();

    if (GetMap(map))
    {
        URHO3D_LOGWARNINGF("Map {%s} already added to the list", map.c_str());
        return;
    }

    MapInfo mapData;
    mapData.map = map;
    mapData.name = eventData[P_NAME].GetString();
    mapData.description = eventData[P_DESCRIPTION].GetString();
    mapData.image = eventData[P_IMAGE].GetString();
    mapData.startPoint = eventData[P_START_POINT].GetVector3();
    mapData.commands = eventData[P_COMMANDS].GetStringVector();
    mapData.startNode = eventData[P_START_NODE].GetString();
    availableMaps_.push_back(mapData);

    URHO3D_LOGINFOF("Map {%s} added to the list", map.c_str());
}

bool SceneManager::CanLoadingStepRun(LoadingStep& loadingStep)
{
    if (loadingStep.finished)
    {
        return false;
    }

    if (!loadingStep.dependsOn.empty())
    {
        for (auto it = loadingStep.dependsOn.begin(); it != loadingStep.dependsOn.end(); ++it)
        {
            eastl::string eventName = (*it);
            if (loadingSteps_.contains(eventName) && !loadingSteps_[eventName].finished)
            {
                return false;
            }
        }
    }

    return true;
}

void SceneManager::CleanupScene()
{
    if (activeScene_)
    {
        activeScene_->SetUpdateEnabled(false);
        activeScene_->Clear();
        activeScene_->Remove();
        activeScene_.Reset();
    }
}

const eastl::vector<MapInfo>& SceneManager::GetAvailableMaps() const { return availableMaps_; }

void SceneManager::LoadDefaultMaps()
{
    using namespace AddMap;
    VariantMap& data = GetEventDataMap();
    {
        data[P_MAP] = "Scenes/Flat.xml";
        data[P_NAME] = "Flatland";
        data[P_DESCRIPTION] = "Flat map where you could easily fall off";
        data[P_IMAGE] = "Textures/flat-platform.png";
        data[P_START_POINT] = Vector3(0, 1, 0);

        StringVector commands;
        commands.push_back("ambient_light 0.5 0.5 0.5");
        commands.push_back("fog 0 100'");
        data[P_COMMANDS] = commands;
        SendEvent(E_ADD_MAP, data);
    }

#ifdef VOXEL_SUPPORT
    {
        // Voxel
        data[P_MAP] = "Scenes/Voxel.xml";
        data[P_NAME] = "Voxel";
        data[P_DESCRIPTION] = "Voxel based map";
        data[P_IMAGE] = "Textures/cube.png";
        data[P_START_POINT] = Vector3(0, 1, 0);

        eastl::stringVector commands;
        commands.Push("ambient_light 0.8 0.8 0.8");
        commands.Push("fog 0 500'");
        //        commands.Push("noclip");
        //        commands.Push("chunk_visible_distance 1");
        //    commands.Push("debugger");
        //    commands.Push("debug_geometry");
        data[P_COMMANDS] = commands;
        SendEvent(E_ADD_MAP, data);
    }
#endif
}

const MapInfo* SceneManager::GetCurrentMapInfo() const { return currentMap_; }
