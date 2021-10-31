
#pragma once

#include "Application/Application.h"
#include <Urho3D/Engine/EventManager.h>

// This is probably always OK.
using namespace Urho3D;

namespace Urho3D
{
struct STAREvents
{
    /// Auto call event
    DEFINE_EVENT(StarAutoCall);
};

URHO3D_EVENT(E_AUTOCALL, AutoCall)
{

}

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

    void AutoCall(StringHash eventType, VariantMap& eventData);

    float prevElapsedTime = 0.f;
    float callInterval = 15.f;
};

} // namespace Urho3D

// A helper macro which defines main function. Forgetting it will result in linker errors complaining about missing
// `_main` or `_WinMain@16`.
URHO3D_DEFINE_APPLICATION_MAIN(InputManagerTest);
