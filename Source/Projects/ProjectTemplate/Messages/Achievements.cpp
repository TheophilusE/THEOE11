#include <Urho3D/Audio/AudioDefs.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/WorkQueue.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/JSONFile.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/ObjectAnimation.h>
#include <Urho3D/Scene/ValueAnimation.h>
#if URHO3D_SYSTEMUI
#include <Urho3D/SystemUI/DebugHud.h>
#endif
#include <Urho3D/IO/FileSystem.h>

#include "../Audio/AudioEvents.h"
#include "../Audio/AudioManagerDefs.h"
#include "../Globals/Settings.h"
#include "Achievements.h"
#include "MessageEvents.h"

using namespace Urho3D;
using namespace AudioEvents;
using namespace MessageEvents;

void SaveProgressAsync(const WorkItem* item, unsigned threadIndex)
{
    Achievements* achievementHandler = reinterpret_cast<Achievements*>(item->aux_);
    achievementHandler->SaveProgress();
}

Achievements::Achievements(Context* context)
    : Object(context)
    , showAchievements_(false)
{
    Init();
}

Achievements::~Achievements() { activeAchievements_.clear(); }

void Achievements::SetShowAchievements(bool show) { showAchievements_ = show; }

void Achievements::Init()
{
    SubscribeToEvents();
    LoadAchievementList();
}

void Achievements::SubscribeToEvents()
{
    SubscribeToEvent(E_NEW_ACHIEVEMENT, URHO3D_HANDLER(Achievements, HandleNewAchievement));
    SubscribeToEvent(E_ADD_ACHIEVEMENT, URHO3D_HANDLER(Achievements, HandleAddAchievement));
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Achievements, HandleUpdate));
}

void Achievements::HandleNewAchievement(StringHash eventType, VariantMap& eventData)
{
    using namespace NewAchievement;
    eastl::string message = eventData[P_MESSAGE].GetString();

    if (!activeAchievements_.empty() || !showAchievements_)
    {
        achievementQueue_.push_back(eventData);
        URHO3D_LOGINFO("Pushing achievement to the queue " + message);
        return;
    }

    for (auto it = activeAchievements_.begin(); it != activeAchievements_.end(); ++it)
    {
        if ((*it)->GetMessage() == message)
        {
            URHO3D_LOGINFO("Achievement already visible!");
            return;
        }
    }

    URHO3D_LOGINFO("New achievement: " + message);

    SharedPtr<SingleAchievement> singleAchievement = context_->CreateObject<SingleAchievement>();

    eastl::string imageName = eventData[P_IMAGE].GetString();
    if (!imageName.empty())
    {
        singleAchievement->SetImage(imageName);
    }
    else
    {
        singleAchievement->SetImage("Textures/UrhoIcon.png");
    }

    singleAchievement->SetMessage(message);
    // Create light animation
    SharedPtr<ObjectAnimation> objAnimation(new ObjectAnimation(context_));

    // Create light position animation
    SharedPtr<ValueAnimation> positionAnimation2(new ValueAnimation(context_));
    // Use spline interpolation method
    positionAnimation2->SetInterpolationMethod(IM_LINEAR);
    // Set spline tension
    // positionAnimation2->SetSplineTension(0.7f);
    positionAnimation2->SetKeyFrame(0.0f, -300.0f);
    positionAnimation2->SetKeyFrame(1.0f, 10.0f);
    positionAnimation2->SetKeyFrame(4.0f, 10.0f);
    positionAnimation2->SetKeyFrame(5.0f, -300.0f);
    positionAnimation2->SetKeyFrame(10.0f, -400.0f);
    objAnimation->AddAttributeAnimation("Offset", positionAnimation2);

    singleAchievement->SetObjectAnimation(objAnimation);
    singleAchievement->SetVar("Lifetime", 5.0f);

    activeAchievements_.push_back(singleAchievement);

    using namespace AudioDefs;
    using namespace PlaySound;
    VariantMap& data = GetEventDataMap();
    data[P_INDEX] = SOUND_EFFECTS::ACHIEVEMENT;
    data[P_TYPE] = SOUND_EFFECT;
    SendEvent(E_PLAY_SOUND, data);

    using namespace AchievementUnlocked;
    VariantMap achievementData = GetEventDataMap();
    achievementData[AchievementUnlocked::P_MESSAGE] = message;
    SendEvent(E_ACHIEVEMENT_UNLOCKED, achievementData);
}

void Achievements::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;

    if (activeAchievements_.empty() && !achievementQueue_.empty() && showAchievements_)
    {
        HandleNewAchievement("", achievementQueue_.front());
        achievementQueue_.pop_front();
    }

    float timeStep = eventData[P_TIMESTEP].GetFloat();
    for (auto it = activeAchievements_.begin(); it != activeAchievements_.end(); ++it)
    {
        if (!(*it))
        {
            activeAchievements_.erase(it);

            if (!achievementQueue_.empty() && showAchievements_)
            {
                HandleNewAchievement("", achievementQueue_.front());
                achievementQueue_.pop_front();
            }

            return;
        }
        float lifetime = (*it)->GetVar("Lifetime").GetFloat();
        if (lifetime <= 0)
        {
            // (*it)->Remove();
            activeAchievements_.erase(it);

            if (!achievementQueue_.empty() && showAchievements_)
            {
                HandleNewAchievement("", achievementQueue_.front());
                achievementQueue_.pop_front();
            }
            return;
        }
        lifetime -= timeStep;
        (*it)->SetVar("Lifetime", lifetime);
    }
}

void Achievements::LoadAchievementList()
{
    LoadProgress();

    auto configFile = GetSubsystem<ResourceCache>()->GetResource<JSONFile>("Config/Achievements.json");

    JSONValue value = configFile->GetRoot();
    if (value.IsArray())
    {
        URHO3D_LOGINFOF("Loading achievements config: %u", value.Size());
        for (int i = 0; i < value.Size(); i++)
        {
            JSONValue mapInfo = value[i];
            if (mapInfo.Contains("Event") && mapInfo["Event"].IsString() && mapInfo.Contains("Image") &&
                mapInfo["Image"].IsString() && mapInfo.Contains("Message") && mapInfo["Message"].IsString() &&
                mapInfo.Contains("Threshold") && mapInfo["Threshold"].IsNumber())
            {

                eastl::string eventName = mapInfo["Event"].GetString();
                eastl::string image = mapInfo["Image"].GetString();
                eastl::string message = mapInfo["Message"].GetString();
                int threshold = mapInfo["Threshold"].GetInt();
                eastl::string parameterName;
                Variant parameterValue;

                if (mapInfo.Contains("ParameterName") && mapInfo["ParameterName"].IsString() &&
                    mapInfo.Contains("Value"))
                {
                    parameterName = mapInfo["ParameterName"].GetString();
                    switch (mapInfo["Value"].GetValueType())
                    {
                    case JSONValueType::JSON_BOOL:
                        parameterValue = mapInfo["Value"].GetBool();
                        break;
                    case JSONValueType::JSON_NUMBER:
                        parameterValue = mapInfo["Value"].GetInt();
                        break;
                    case JSONValueType::JSON_STRING:
                        parameterValue = mapInfo["Value"].GetString();
                        break;
                    default:
                        continue;
                    }
                }

                AddAchievement(message, eventName, image, threshold, parameterName, parameterValue);
            }
            else
            {
                URHO3D_LOGINFOF("Achievement array element doesnt contain all needed info! Index: %u", i);
            }
        }

        if (GetSubsystem<DebugHud>())
        {
            GetSubsystem<DebugHud>()->SetAppStats("Total achievements loaded", CountAchievements());
        }
    }
    else
    {
        URHO3D_LOGERROR("Data/Config/Achievements.json must be an array");
    }
}

void Achievements::HandleRegisteredEvent(StringHash eventType, VariantMap& eventData)
{
    bool processed = false;
    if (registeredAchievements_.contains(eventType))
    {
        for (auto it = registeredAchievements_[eventType].begin(); it != registeredAchievements_[eventType].end(); ++it)
        {
            if ((*it).deepCheck)
            {
                // check if the event contains specified parameter and same value
                if (eventData[(*it).parameterName] == (*it).parameterValue)
                {
                    (*it).current++;
                    processed = true;
                }
            }
            else
            {
                // No additional check needed, event was called, so we can increment our counter
                (*it).current++;
                processed = true;
            }

            // URHO3D_LOGINFOF("Achievement progress: '%s' => %i/%i",(*it).message.CString(), (*it).current,
            // (*it).threshold);
            if ((*it).current >= (*it).threshold && !(*it).completed)
            {
                (*it).completed = true;
                VariantMap& data = GetEventDataMap();
                data["Message"] = (*it).message;
                data["Image"] = (*it).image;
                SendEvent(E_NEW_ACHIEVEMENT, data);
            }
        }
    }

    // If any of the achievements were updated, save progress
    if (processed)
    {
        WorkQueue* workQueue = GetSubsystem<WorkQueue>();
        SharedPtr<WorkItem> item = workQueue->GetFreeItem();
        item->priority_ = M_MAX_UNSIGNED;
        item->workFunction_ = SaveProgressAsync;
        item->aux_ = this;
        // send E_WORKITEMCOMPLETED event after finishing WorkItem
        item->sendEvent_ = true;

        item->start_ = nullptr;
        item->end_ = nullptr;
        workQueue->AddWorkItem(item);
    }
}

eastl::list<AchievementRule> Achievements::GetAchievements()
{
    achievements_.clear();
    for (auto it = registeredAchievements_.begin(); it != registeredAchievements_.end(); ++it)
    {
        for (auto it2 = (*it).second.begin(); it2 != (*it).second.end(); ++it2)
        {
            achievements_.push_back((*it2));
        }
    }

    return achievements_;
}

void Achievements::SaveProgress()
{
    JSONFile file(context_);
    for (auto it = registeredAchievements_.begin(); it != registeredAchievements_.end(); ++it)
    {
        for (auto achievement = (*it).second.begin(); achievement != (*it).second.end(); ++achievement)
        {
            StringHash id = (*achievement).eventName + (*achievement).message;
            file.GetRoot()[id.ToString()] = (*achievement).current;
        }
    }
#if defined(__ANDROID__)
    String directory = GetSubsystem<FileSystem>()->GetUserDocumentsDir() + DOCUMENTS_DIR;
    file.SaveFile(directory + "/Achievements.json");
#elif defined(__EMSCRIPTEN__)
    // TODO: implement local storage utilization for web
#else
    file.SaveFile(GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Saves/Achievements.json");
#endif
}

void Achievements::LoadProgress()
{
    JSONFile configFile(context_);

#if defined(__ANDROID__)
    String directory = GetSubsystem<FileSystem>()->GetUserDocumentsDir() + DOCUMENTS_DIR;
    configFile.LoadFile(directory + "/Achievements.json");
#elif defined(__EMSCRIPTEN__)
    // TODO: implement achievement progress loading
#else
    configFile.LoadFile(GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Saves/Achievements.json");
#endif

    JSONValue value = configFile.GetRoot();
    if (value.IsObject())
    {
        for (auto& it : value)
        {
            progress_[(it).first] = (it).second.GetInt();
        }
    }
}

void Achievements::ClearAchievementsProgress()
{
    for (auto it = registeredAchievements_.begin(); it != registeredAchievements_.end(); ++it)
    {
        for (auto achievement = (*it).second.begin(); achievement != (*it).second.end(); ++achievement)
        {
            achievement->current = 0;
            achievement->completed = false;
        }
    }

    SaveProgress();
}

void Achievements::AddAchievement(eastl::string message, eastl::string eventName, eastl::string image, int threshold,
                                  eastl::string parameterName, Variant parameterValue)
{
    AchievementRule rule;
    rule.message = message;
    rule.eventName = eventName;
    rule.image = image;
    rule.threshold = threshold;
    rule.current = 0;
    rule.completed = false;
    if (!parameterName.empty() && !parameterValue.IsEmpty())
    {
        rule.deepCheck = true;
    }
    else
    {
        rule.deepCheck = false;
    }
    rule.parameterName = parameterName;
    rule.parameterValue = parameterValue;

    // Check current achievement saved progress
    StringHash id = rule.eventName + rule.message;
    if (progress_.contains(id.ToString()))
    {
        rule.current = progress_[id.ToString()];

        // Check if achievement was already unlocked
        if (rule.current >= rule.threshold)
        {
            rule.completed = true;
            URHO3D_LOGINFO("Achievement '{}' already unlocked!", rule.message);
        }
    }

    registeredAchievements_[eventName].push_back(rule);

    //    URHO3D_LOGINFOF("Registering achievement [%s]", rule.message.CString());

    SubscribeToEvent(eventName, URHO3D_HANDLER(Achievements, HandleRegisteredEvent));

    if (GetSubsystem<DebugHud>())
    {
        GetSubsystem<DebugHud>()->SetAppStats("Total achievements loaded", CountAchievements());
    }
}

void Achievements::HandleAddAchievement(StringHash eventType, VariantMap& eventData)
{
    using namespace AddAchievement;
    if (eventData.contains(P_EVENT) && eventData.contains(P_MESSAGE) && eventData.contains(P_IMAGE) &&
        eventData.contains(P_THRESHOLD))
    {
        AddAchievement(eventData[P_MESSAGE].GetString(), eventData[P_EVENT].GetString(), eventData[P_IMAGE].GetString(),
                       eventData[P_THRESHOLD].GetInt(), eventData[P_PARAMETER_NAME].GetString(),
                       eventData[P_PARAMETER_VALUE]);
    }
    else
    {
        URHO3D_LOGERRORF("Unable to register achievement {%s}, incomplete data provided!",
                         eventData[P_MESSAGE].GetString().c_str());
    }
}

int Achievements::CountAchievements()
{
    int count = 0;
    for (auto it = registeredAchievements_.begin(); it != registeredAchievements_.end(); ++it)
    {
        count += (*it).second.size();
    }
    return count;
}
