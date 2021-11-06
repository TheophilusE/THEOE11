#pragma once

#include "../BaseLevel.h"
#include <Urho3D/UI/UIElement.h>

namespace Levels
{

class Credits : public BaseLevel
{
    URHO3D_OBJECT(Credits, BaseLevel);

public:
    Credits(Context* context);
    virtual ~Credits();
    static void RegisterObject(Context* context);

protected:
    void Init() override;

private:
    void CreateScene();

    /**
     * Create the actual Credits content
     */
    void CreateUI();

    void SubscribeToEvents();

    /**
     * End credits
     */
    void HandleEndCredits(bool forced = false);

    UIElement* CreateEmptyLine(int height);

    /**
     * Create single text line
     */
    void CreateSingleLine(eastl::string content, int fontSize);

    /**
     * Create single image line
     */
    void CreateImageLine(const eastl::string& image, int size);

    /**
     * Handle credits scrolling
     */
    void HandleUpdate(StringHash eventType, VariantMap& eventData);

    /**
     * Credits window lifetime timer
     */
    Timer timer_;

    /**
     * Credits view content
     */
    eastl::vector<SharedPtr<UIElement>> credits_;

    /**
     * Credits base UI view
     */
    SharedPtr<UIElement> creditsBase_;

    float offset_;
};
} // namespace Levels
