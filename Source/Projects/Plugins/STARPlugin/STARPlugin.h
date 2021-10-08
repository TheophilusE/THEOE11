
#pragma once


#include <Urho3D/Engine/PluginApplication.h>

// Forward Declarations
class Character;


namespace Urho3D
{

class STARPlugin : public PluginApplication
{
    URHO3D_OBJECT(STARPlugin, PluginApplication);
public:
    /// Construct.
    explicit STARPlugin(Context* context);

    /// Initialize plugin.
    void Load() override;
    /// Start game.
    void Start() override;
    /// Stop game.
    void Stop() override;
    /// Deinitialize plugin.
    void Unload() override;

private:
    /// Register scene components to application context
    void RegisterObjects();
    /// Subscribe to necessary events.
    void SubscribeToEvents();
    /// Unsubscribe to necessary events.
    void UnsubscribeToEvents();
    /// Handle application update. Set controls to character.
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    /// Handle application post-update. Update camera position after character has moved.
    void HandlePostUpdate(StringHash eventType, VariantMap& eventData);
    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData);

};


}
