#pragma once

#include "../BaseWindow.h"
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Window.h>

class PauseWindow : public BaseWindow
{
    URHO3D_OBJECT(PauseWindow, BaseWindow);

public:
    PauseWindow(Context* context);

    virtual ~PauseWindow();

    virtual void Init() override;

protected:
    virtual void Create() override;

private:
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void SubscribeToEvents();

    SharedPtr<Button> continueButton_;
    SharedPtr<Button> mainMenuButton_;
    SharedPtr<Button> settingsButton_;
    SharedPtr<Button> exitButton_;
    SharedPtr<Window> baseWindow_;

    Button* CreateButton(const eastl::string& text, int width, IntVector2 position);
};
