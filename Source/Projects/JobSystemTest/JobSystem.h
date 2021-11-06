//
// Copyright (c) 2008-2020 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include <FiberJobSystem/Include/FJS/Counter.h>
#include <FiberJobSystem/Include/FJS/List.h>
#include <FiberJobSystem/Include/FJS/Manager.h>
#include <FiberJobSystem/Include/FJS/Queue.h>
#include <EASTL/unique_ptr.h>

#include <Urho3D/IO/Log.h>

namespace Urho3D
{
class JobSystem
{
    JobSystem();
    JobSystem(JobSystem&) = delete;
    ~JobSystem() = default;

    eastl::unique_ptr<FJS::Manager> m_Manager;
    FJS::ManagerOptions m_ManagerOptions;

    using Main_t = void(*)(FJS::Manager*);

public:
    static JobSystem* GetSingleton();
    /// Run Main Function that implements the Job System Manager
    static bool Run(Main_t main) { return GetSingleton()->Run_(main); }
    /// Schedule job to be executed
    static void ScheduleJob(FJS::JobPriority priority, const FJS::JobInfo& info)
    {
        return GetSingleton()->ScheduleJob_(priority, info);
    }
    static void ShutDown(bool blocking) { return GetSingleton()->ShutDown_(blocking); }
    static FJS::Manager* GetManager() { return GetSingleton()->m_Manager.get(); }

private:
    bool Run_(Main_t main);
    void ScheduleJob_(FJS::JobPriority priority, const FJS::JobInfo& info);
    void ShutDown_(bool blocking);
};
} // namespace Urho3D
