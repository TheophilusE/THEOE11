#pragma once

#include <Urho3D/Container/Str.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Resource/JSONFile.h>

using namespace Urho3D;

class State : public Object
{
    URHO3D_OBJECT(State, Object);

    static void RegisterObject(Context* context);

public:
    State(Context* context);

    virtual ~State();

    void SetValue(const eastl::string& name, const Variant& value, bool save = false);
    const Variant& GetValue(const eastl::string& name);

private:
    void SubscribeToEvents();
    void Load();
    void Save();
    void HandleSetParameter(StringHash eventType, VariantMap& eventData);
    void HandleIncrementParameter(StringHash eventType, VariantMap& eventData);
    eastl::string fileLocation_;

    VariantMap data_;
};
