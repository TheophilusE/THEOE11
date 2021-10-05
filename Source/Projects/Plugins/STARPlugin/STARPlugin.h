
#pragma once


#include <Urho3D/Engine/PluginApplication.h>


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
};


}
