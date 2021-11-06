
#pragma once

#include "BaseApplication.h"

using namespace Urho3D;

class Star : public BaseApplication
{
    URHO3D_OBJECT(Star, BaseApplication);

public:
    Star(Context* context);

    /**
     * Setup before engine initialization
     */
    virtual void Setup() override;
    /**
     * Setup after engine initialization
     */
    virtual void Start() override;

    /**
     * Cleanup after the main loop
     */
    virtual void Stop() override;

private:

    /// Handle application update.
    void HandleUpdate(StringHash eventType, VariantMap& eventData);

    void SubscribeToEvents();
};

URHO3D_DEFINE_APPLICATION_MAIN(Star);
