
#include "JobSystem.h"

namespace Urho3D
{
JobSystem::JobSystem()
{
    m_ManagerOptions.NumFibers = m_ManagerOptions.NumThreads * 10;
    m_ManagerOptions.ThreadAffinity = true;
    m_ManagerOptions.ShutdownAfterMainCallback = true;

    m_Manager.reset(new FJS::Manager(m_ManagerOptions));
}

JobSystem* JobSystem::GetSingleton()
{
    static JobSystem singleton;
    return &singleton;
}

bool JobSystem::Run_(Main_t main)
{
    FJS::Manager::ReturnCode retCode = m_Manager->Run(main);

    if (retCode == FJS::Manager::ReturnCode::Succes)
    {
        URHO3D_LOGINFO("Execution Success");
        return true;
    }
    else if (retCode == FJS::Manager::ReturnCode::AlreadyInitialized)
    {
        URHO3D_LOGERROR("Execution Already Initialized");
        return false;
    }
    else if (retCode == FJS::Manager::ReturnCode::ErrorThreadAffinity)
    {
        URHO3D_LOGERROR("Execution Error Thread Affinity");
        return false;
    }
    else if (retCode == FJS::Manager::ReturnCode::InvalidNumFibers)
    {
        URHO3D_LOGERROR("Execution Error Invalid Number Of Fibers");
        return false;
    }
    else if (retCode == FJS::Manager::ReturnCode::NullCallback)
    {
        URHO3D_LOGERROR("Execution Error Null Callback");
        return false;
    }
    else if (retCode == FJS::Manager::ReturnCode::OSError)
    {
        URHO3D_LOGERROR("Execution OS Error");
        return false;
    }
    else
    {
        URHO3D_LOGERROR("Execution Error Unknown");
        return false;
    }
}

void JobSystem::ScheduleJob_(FJS::JobPriority priority, const FJS::JobInfo& info)
{
    m_Manager->ScheduleJob(priority, info);
}
}
