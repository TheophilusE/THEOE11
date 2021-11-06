#pragma once

#include "../../Config/ConfigFile.h"
#include "BaseInput.h"

class KeyboardInput : public BaseInput
{
    URHO3D_OBJECT(KeyboardInput, BaseInput);

public:
    KeyboardInput(Context* context)
        : BaseInput(context)
    {
    }

    // virtual ~KeyboardInput();
    virtual eastl::string GetActionKeyName(int action) override { return "";  }

protected:
    virtual void Init() {}

private:
    void SubscribeToEvents();

    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
    void HandleKeyUp(StringHash eventType, VariantMap& eventData);
};
