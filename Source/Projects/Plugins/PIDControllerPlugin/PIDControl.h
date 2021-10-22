//
// Copyright (c) 2017-2020 the rbfx project.
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

#include <Urho3D/Core/Context.h>
#include <Urho3D/Engine/PluginApplication.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Math/PIDControl.h>
#include <Urho3D/Scene/Node.h>

namespace Urho3D
{
enum class PID_UPDATE
{
    UpdateAsPID = 0,
    UpdateAsPI,
    UpdateAsPD,
    UpdateAsP,
    UpdateAuto,
    DoNotUpdate
};

/// Character component, responsible for physical movement according to controls, as well as animation.
class PIDControl : public LogicComponent
{
    URHO3D_OBJECT(PIDControl, LogicComponent);

public:
    /// Construct.
    explicit PIDControl(Context* context);

    /// Register object factory and attributes.
    static void RegisterObject(Context* context);
    static void RegisterObject(Context* context, PluginApplication* plugin);

    /// Handle startup. Called by LogicComponent base class.
    void Start() override;
    /// Handle physics world update. Called by LogicComponent base class.
    void Update(float timeStep) override;

private:

    FPIDController m_PID;
    PID_UPDATE m_PIDUpdate;
    float inError;
    static const char* PIDData[];
};

} // namespace Urho3D
