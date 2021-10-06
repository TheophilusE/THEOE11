
#pragma once

#include <Urho3D/Core/Context.h>
#include <Urho3D/Engine/PluginApplication.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Scene/Node.h>

namespace Urho3D
{

/// A custom component provided by the plugin.
class PlayerCameraManager : public LogicComponent
{
    URHO3D_OBJECT(PlayerCameraManager, LogicComponent);

public:
    PlayerCameraManager(Context* context)
        : LogicComponent(context)
    {
        SetUpdateEventMask(USE_UPDATE);
        SetUpdateEventMask(USE_POSTUPDATE);
    }

    void Update(float timeStep) override;
    void PostUpdate(float timeStep) override;

    static void RegisterObject(Context* context, PluginApplication* plugin);

    struct
    {
        float minDistance = 1.0f;
        float initialDistance = 5.0f;
        float maxDistance = 20.0f;

        bool firstPerson = true;

    } m_Camera;
};

} // namespace Urho3D
