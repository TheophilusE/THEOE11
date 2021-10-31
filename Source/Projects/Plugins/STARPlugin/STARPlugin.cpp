
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Input/Controls.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>

#include <Urho3D/Core/Context.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Core/Timer.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/IO/Log.h>

#include "STARPlugin.h"
#include "Locomotion/Character/BaseCharacter.h"
#include "Locomotion/Camera/PlayerCameraManager.h"
#include "Locomotion/Character/CharacterController.h"

URHO3D_DEFINE_PLUGIN_MAIN(Urho3D::STARPlugin);


namespace Urho3D
{

STARPlugin::STARPlugin(Context* context)
    : PluginApplication(context)
{
}

void STARPlugin::RegisterObjects()
{
    // Register factory and attributes for the Character component so it can be created via CreateComponent, and
    // loaded / saved
    Character::RegisterObject(context_, this);
    PlayerCameraManager::RegisterObject(context_, this);
    CharacterController::RegisterObject(context_, this);
}

void STARPlugin::Load()
{
    // Register custom components/subsystems/events when plugin is loaded.
    //RotateObject::RegisterObject(context_, this);
    RegisterObjects();
}

void STARPlugin::Unload()
{
    // Finalize plugin, ensure that no objects provided by the plugin are alive. Some of that work is automated by
    // parent class. Objects that had factories registered through PluginApplication::RegisterFactory<> have their
    // attributes automatically unregistered, factories/subsystems removed.
}

void STARPlugin::Start()
{
    // Set up any game state here. Configure input. Create objects. Add UI. Game application assumes control of the
    // input.
    //context_->GetSubsystem<Input>()->SetMouseVisible(false);
    //context_->GetSubsystem<Input>()->SetMouseMode(MM_WRAP);

    URHO3D_LOGINFO("Star Plugin -> On Start!");

    SubscribeToEvents();
}

void STARPlugin::Stop()
{
    // Tear down any game state here. Unregister events. Remove objects. Editor takes back the control.

    UnsubscribeToEvents();

    URHO3D_LOGINFO("Star Plugin -> On Stop");
}

void STARPlugin::SubscribeToEvents()
{
    // Subscribe to Update event for setting the character controls before physics simulation
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(STARPlugin, HandleUpdate));
    // Subscribe to PostUpdate event for updating the camera position after physics simulation
    SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(STARPlugin, HandlePostUpdate));
    SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(STARPlugin, HandlePostRenderUpdate));
}

void STARPlugin::UnsubscribeToEvents()
{
    UnsubscribeFromEvent(this, E_UPDATE);
    UnsubscribeFromEvent(this, E_POSTUPDATE);
    UnsubscribeFromEvent(this, E_POSTRENDERUPDATE);
}

void STARPlugin::HandleUpdate(StringHash eventType, VariantMap& eventData) {}
void STARPlugin::HandlePostUpdate(StringHash eventType, VariantMap& eventData) {}
void STARPlugin::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData) {}

}
