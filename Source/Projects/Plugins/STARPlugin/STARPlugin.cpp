
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Core/Timer.h>
#include <Urho3D/Scene/Node.h>


URHO3D_DEFINE_PLUGIN_MAIN(Urho3D::STARPlugin);


namespace Urho3D
{

STARPlugin::STARPlugin(Context* context)
    : PluginApplication(context)
{
}

void STARPlugin::Load()
{
    // Register custom components/subsystems/events when plugin is loaded.
    //RotateObject::RegisterObject(context_, this);
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
    context_->GetSubsystem<Input>()->SetMouseVisible(false);
    context_->GetSubsystem<Input>()->SetMouseMode(MM_WRAP);
}

void STARPlugin::Stop()
{
    // Tear down any game state here. Unregister events. Remove objects. Editor takes back the control.
}

}
