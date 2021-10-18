
#pragma once

#include "Application/Application.h"

// This is probably always OK.
using namespace Urho3D;

class InputManagerTest : public App
{
    // This macro defines some methods that every `Urho3D::Object` descendant should have.
    URHO3D_OBJECT(InputManagerTest, App);

public:
    // Likewise every `Urho3D::Object` descendant should implement constructor with single `Context*` parameter.
    InputManagerTest(Context* context)
        : App(context)
    {
    }

    void Setup() override;

    void Start() override;

    void Stop() override;

private:
    void SubscribeToEvents();
    /// Handle application update. Set controls to character.
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    /// Handle application post-update. Update camera position after character has moved.
    void HandlePostUpdate(StringHash eventType, VariantMap& eventData);
    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData);
};

// A helper macro which defines main function. Forgetting it will result in linker errors complaining about missing
// `_main` or `_WinMain@16`.
URHO3D_DEFINE_APPLICATION_MAIN(InputManagerTest);
