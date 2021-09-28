
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Input/Input.h>

// This is probably always OK.
using namespace Urho3D;


class STAR : public Application
{
    // This macro defines some methods that every `Urho3D::Object` descendant should have.
    URHO3D_OBJECT(STAR, Application);
public:
    // Likewise every `Urho3D::Object` descendant should implement constructor with single `Context*` parameter.
    STAR(Context* context)
        : Application(context)
    {
    }

    void Setup() override;

    void Start() override;

    void Stop() override;
};

// A helper macro which defines main function. Forgetting it will result in linker errors complaining about missing `_main` or `_WinMain@16`.
URHO3D_DEFINE_APPLICATION_MAIN(STAR);