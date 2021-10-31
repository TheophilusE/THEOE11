
#include "PIDControl.h"

namespace Urho3D
{
const char* PIDControl::PIDData[] = {"UpdateAsPID", "UpdateAsPI", "UpdateAsPD", "UpdateAsP", "UpdateAuto", "DoNotUpdate", nullptr};

PIDControl::PIDControl(Context* context)
    : LogicComponent(context)
    , m_PIDUpdate(PID_UPDATE::UpdateAsPID)
    , inError(0.0f)
{
    // Only the physics update event is needed: unsubscribe from the rest for optimization
    SetUpdateEventMask(USE_UPDATE);
}

void PIDControl::RegisterObject(Context* context)
{
    context->RegisterFactory<PIDControl>();

    // These macros register the class attributes to the Context for automatic load / save handling.
    // We specify the Default attribute mode which means it will be used both for saving into file, and network
    // replication
    URHO3D_ATTRIBUTE("P", float, m_PID.P, 0.0f, AM_DEFAULT);
    URHO3D_ATTRIBUTE("I", float, m_PID.I, 0.0f, AM_DEFAULT);
    URHO3D_ATTRIBUTE("D", float, m_PID.D, 0.0f, AM_DEFAULT);
    URHO3D_ATTRIBUTE("Max Output", float, m_PID.MaxOutAbs, 1000.0f, AM_DEFAULT);
    URHO3D_ATTRIBUTE("In Error", float, inError, 0.0f, AM_DEFAULT);
    URHO3D_ENUM_ATTRIBUTE("Update Type", m_PIDUpdate, PIDData, PID_UPDATE::UpdateAsPID, AM_DEFAULT);
}

void PIDControl::RegisterObject(Context* context, PluginApplication* plugin)
{
    plugin->RegisterFactory<PIDControl>("STAR Plugin");

    // These macros register the class attributes to the Context for automatic load / save handling.
    // We specify the Default attribute mode which means it will be used both for saving into file, and network
    // replication
    URHO3D_ATTRIBUTE("P", float, m_PID.P, 0.0f, AM_DEFAULT);
    URHO3D_ATTRIBUTE("I", float, m_PID.I, 0.0f, AM_DEFAULT);
    URHO3D_ATTRIBUTE("D", float, m_PID.D, 0.0f, AM_DEFAULT);
    URHO3D_ATTRIBUTE("Max Output", float, m_PID.MaxOutAbs, 1000.0f, AM_DEFAULT);
    URHO3D_ATTRIBUTE("In Error", float, inError, 0.0f, AM_DEFAULT);
    URHO3D_ENUM_ATTRIBUTE("Update Type", m_PIDUpdate, PIDData, PID_UPDATE::UpdateAsPID, AM_DEFAULT);
}

void PIDControl::Start()
{
    // Component has been inserted into its scene node. Subscribe to events now
}

void PIDControl::Update(float timeStep)
{
    switch (m_PIDUpdate)
    {
    case Urho3D::PID_UPDATE::UpdateAsPID:
        m_PID.UpdateAsPID(inError, timeStep);
        break;
    case Urho3D::PID_UPDATE::UpdateAsPI:
        m_PID.UpdateAsPI(inError, timeStep);
        break;
    case Urho3D::PID_UPDATE::UpdateAsPD:
        m_PID.UpdateAsPD(inError, timeStep);
        break;
    case Urho3D::PID_UPDATE::UpdateAsP:
        m_PID.UpdateAsP(inError, timeStep);
        break;
    case Urho3D::PID_UPDATE::UpdateAuto:
        m_PID.Update(inError, timeStep);
        break;
    case Urho3D::PID_UPDATE::DoNotUpdate:
        break;
    default:
        break;
    }
}

} // namespace Urho3D
