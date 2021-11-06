#pragma once

#include "../BaseWindow.h"
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/CheckBox.h>
#include <Urho3D/UI/Window.h>

class NewGameSettingsWindow : public BaseWindow
{
    URHO3D_OBJECT(NewGameSettingsWindow, BaseWindow);

public:
    NewGameSettingsWindow(Context* context);

    virtual ~NewGameSettingsWindow();

    virtual void Init() override;

protected:
    virtual void Create() override;

private:
    void SubscribeToEvents();

    Button* CreateButton(UIElement* parent, const eastl::string& text, int width, IntVector2 position);
    CheckBox* CreateCheckbox(const eastl::string& label);

    void CreateLevelSelection();

    SharedPtr<Window> baseWindow_;
    SharedPtr<UIElement> levelSelection_;
    SharedPtr<CheckBox> startServer_;
    SharedPtr<CheckBox> connectServer_;
};
