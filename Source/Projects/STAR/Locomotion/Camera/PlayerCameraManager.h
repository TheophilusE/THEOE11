
#pragma once

#include <Urho3D/Core/Context.h>
#include <Urho3D/Engine/PluginApplication.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Scene/Node.h>

class Character;
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
        //SetUpdateEventMask(USE_UPDATE);
        //SetUpdateEventMask(USE_POSTUPDATE);
    }

    void Start() override;
    void Update(float timeStep) override;
    void PostUpdate(float timeStep) override;
    void SetUp(Scene* scene, Node* cameraNode);

    static void RegisterObject(Context* context, PluginApplication* plugin);
    static void RegisterObject(Context* context);

private:
    /// The controllable character component.
    WeakPtr<Character> character_;
    /// Scene.
    SharedPtr<Scene> scene_;
    /// Camera scene node.
    SharedPtr<Node> cameraNode_;

    struct
    {
        float minDistance = 1.0f;
        float initialDistance = 5.0f;
        float maxDistance = 20.0f;

        float pitch = 0.0f;
        float yaw = 0.0f;

        bool firstPerson = true;

    } m_Camera;
};

} // namespace Urho3D
