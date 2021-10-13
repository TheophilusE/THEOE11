
#pragma once

#include "Application/Application.h"

// This is probably always OK.
using namespace Urho3D;

class Character;

class STAR : public App
{
    // This macro defines some methods that every `Urho3D::Object` descendant should have.
    URHO3D_OBJECT(STAR, App);

public:
    // Likewise every `Urho3D::Object` descendant should implement constructor with single `Context*` parameter.
    STAR(Context* context)
        : App(context)
    {
        RegisterObjects(context);
    }

    void Setup() override;

    void Start() override;

    void Stop() override;

private:
    void RegisterObjects(Context* context);
    /// Create static scene content.
    void CreateScene();
    /// Create controllable character.
    void CreateCharacter();
    /// Construct an instruction text to the UI.
    void CreateInstructions();
    /// Subscribe to necessary events.
    void SubscribeToEvents();
    /// Handle application update. Set controls to character.
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    /// Handle application post-update. Update camera position after character has moved.
    void HandlePostUpdate(StringHash eventType, VariantMap& eventData);
    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData);

    /// The controllable character component.
    WeakPtr<Character> character_;
    bool drawDebug_ = false;
};

// A helper macro which defines main function. Forgetting it will result in linker errors complaining about missing
// `_main` or `_WinMain@16`.
URHO3D_DEFINE_APPLICATION_MAIN(STAR);
