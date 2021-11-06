#pragma once

#include "../BaseWindow.h"
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Window.h>

class PopupMessageWindow : public BaseWindow
{
    URHO3D_OBJECT(PopupMessageWindow, BaseWindow);

public:
    PopupMessageWindow(Context* context);

    virtual ~PopupMessageWindow();

    virtual void Init() override;

protected:
    virtual void Create() override;

private:
    void SubscribeToEvents();

    SharedPtr<Button> okButton_;
    SharedPtr<Window> baseWindow_;

    Button* CreateButton(const eastl::string& text, int width, IntVector2 position);
    Text* CreateLabel(const eastl::string& text, int fontSize);
};
