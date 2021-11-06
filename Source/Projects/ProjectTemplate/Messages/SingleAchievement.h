#pragma once

#include <Urho3D/Scene/Animatable.h>
#include <Urho3D/UI/Sprite.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Window.h>

using namespace Urho3D;

class SingleAchievement : public Animatable
{
    URHO3D_OBJECT(SingleAchievement, Animatable);

public:
    SingleAchievement(Context* context);

    virtual ~SingleAchievement();

    static void RegisterObject(Context* context);

    /**
     * Set achievement image
     */
    void SetImage(eastl::string image);

    /**
     * Set achievement message
     */
    void SetMessage(eastl::string message);

    /**
     * Get achievement message
     */
    eastl::string GetMessage();

    /**
     * Set object variable
     */
    void SetVar(StringHash key, const Variant& value);

    /**
     * Get object variable
     */
    const Variant& GetVar(const StringHash& key) const;

private:
    void HandlePostUpdate(StringHash eventType, VariantMap& eventData);
    /**
     * Handle attribute animation added.
     */
    void OnAttributeAnimationAdded() override;

    /**
     * Handle attribute animation removed.
     */
    void OnAttributeAnimationRemoved() override;

    /**
     * X position on the screen
     * Animated over time
     */
    float offset_;

    /**
     * Achievement title
     */
    eastl::string message_;

    /**
     * Object variables
     */
    VariantMap vars_;

    /**
     * Achievement item window
     */
    SharedPtr<Window> baseWindow_;

    /**
     * Achievement item image
     */
    SharedPtr<Sprite> sprite_;

    /**
     * Achievement item text
     */
    SharedPtr<Text> title_;
};
