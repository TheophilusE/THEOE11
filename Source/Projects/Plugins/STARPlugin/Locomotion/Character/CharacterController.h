
#pragma once

#include <Urho3D/Core/Context.h>
#include <Urho3D/Engine/PluginApplication.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Scene/Node.h>

namespace Urho3D
{

/// A custom component provided by the plugin.
class CharacterController : public LogicComponent
{
    URHO3D_OBJECT(CharacterController, LogicComponent);

public:
    CharacterController(Context* context)
        : LogicComponent(context)
    {
        //SetUpdateEventMask(USE_UPDATE);
        //SetUpdateEventMask(USE_POSTUPDATE);
    }

    void Start() override;
    void Update(float timeStep) override;
    void PostUpdate(float timeStep) override;

    static void RegisterObject(Context* context, PluginApplication* plugin);
    
};

} // namespace Urho3D
