#pragma once

#include "../BaseLevel.h"
#include <Urho3D/UI/Button.h>

using namespace Urho3D;

namespace Levels
{

class MainMenu : public BaseLevel
{
    URHO3D_OBJECT(MainMenu, BaseLevel);

public:
    MainMenu(Context* context);
    ~MainMenu();
    static void RegisterObject(Context* context);

protected:
    void Init() override;

private:
    void CreateScene();

    void CreateUI();

    void SubscribeToEvents();

    void AddButton(const eastl::string& buttonName, const eastl::string& label, const eastl::string& name,
                   const eastl::string& eventToCall);

    void HandleUpdate(StringHash eventType, VariantMap& eventData);

    void InitCamera();

    SharedPtr<Node> cameraRotateNode_;
    SharedPtr<UIElement> buttonsContainer_;
    eastl::list<SharedPtr<Button>> dynamicButtons_;

    Button* CreateButton(const eastl::string& text);
};
} // namespace Levels
