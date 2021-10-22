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

#include "PIDControl.h"
#include "Urho3D/IO/Log.h"
#include <Urho3D/Physics/RigidBody.h>

namespace Urho3D
{

/// Character component, responsible for physical movement according to controls, as well as animation.
class PIDTranslator : public LogicComponent
{
    URHO3D_OBJECT(PIDTranslator, LogicComponent);

public:
    /// Construct.
    explicit PIDTranslator(Context* context)
        : LogicComponent(context)
    {
        // Only the physics update event is needed: unsubscribe from the rest for optimization
        // SetUpdateEventMask(USE_FIXEDUPDATE);
    }

    /// Register object factory and attributes.
    static void RegisterObject(Context* context)
    {
        context->RegisterFactory<PIDTranslator>();

        // These macros register the class attributes to the Context for automatic load / save handling.
        // We specify the Default attribute mode which means it will be used both for saving into file, and network
        // replication
        URHO3D_ATTRIBUTE("Animate", bool, animate_, false, AM_EDIT);
    }
    static void RegisterObject(Context* context, PluginApplication* plugin)
    {
        plugin->RegisterFactory<PIDTranslator>("STAR Plugin");

        // These macros register the class attributes to the Context for automatic load / save handling.
        // We specify the Default attribute mode which means it will be used both for saving into file, and network
        // replication
        URHO3D_ATTRIBUTE("Animate", bool, animate_, false, AM_EDIT);

        //URHO3D_ATTRIBUTE("x_Axis_P", float, _xAxisP, 0.5f, AM_EDIT);
        //URHO3D_ATTRIBUTE("x_Axis_I", float, _xAxisI, 0.3f, AM_EDIT);
        //URHO3D_ATTRIBUTE("x_Axis_D", float, _xAxisD, 0.2f, AM_EDIT);

        //URHO3D_ATTRIBUTE("y_Axis_P", float, _yAxisP, 0.5f, AM_EDIT);
        //URHO3D_ATTRIBUTE("y_Axis_I", float, _yAxisI, 0.3f, AM_EDIT);
        //URHO3D_ATTRIBUTE("y_Axis_D", float, _yAxisD, 0.2f, AM_EDIT);

        //URHO3D_ATTRIBUTE("z_Axis_P", float, _zAxisP, 0.5f, AM_EDIT);
        //URHO3D_ATTRIBUTE("z_Axis_I", float, _zAxisI, 0.3f, AM_EDIT);
        //URHO3D_ATTRIBUTE("z_Axis_D", float, _zAxisD, 0.2f, AM_EDIT);

        URHO3D_ATTRIBUTE("Thrust", float, thrust, 0.f, AM_EDIT);
    }

    /// Handle startup. Called by LogicComponent base class.
    void DelayedStart() override
    {
        x_Axis.Init(_xAxisP, _xAxisI, _xAxisD, 1000.f);
        y_Axis.Init(_yAxisP, _yAxisI, _yAxisD, 1000.f);
        z_Axis.Init(_zAxisP, _zAxisI, _zAxisD, 1000.f);
    }

    /// Handle physics world update. Called by LogicComponent base class.
    void FixedUpdate(float timeStep) override
    {
        auto* target = GetNode()->GetParent()->GetChild("TargetNode");
        auto* rigidBody = GetNode()->GetComponent<RigidBody>();

        if (animate_)
            GetNode()->Rotate(Quaternion(10 * timeStep, 20 * timeStep, 30 * timeStep));
        else
        {
            // Handle rotation
            Vector3 one = GetNode()->GetTransform().Translation();
            Vector3 two = target->GetTransform().Translation();
            one.Normalize();
            two.Normalize();

            Quaternion targetRotation = GetNode()->GetTransform().Rotation();
            targetRotation.RotateTowards(target->GetTransform().Rotation(), 360.f);
            // GetNode()->SetRotation(targetRotation);

            // PID
            // Figure out the error for each axes {.5, .3, .2}
            float xAngleError =
                DeltaAngle(GetNode()->GetTransform().Rotation().EulerAngles().x_, targetRotation.EulerAngles().x_);
            float xTorqueCorrection = x_Axis.UpdateAsPID(xAngleError, timeStep);

            float yAngleError =
                DeltaAngle(GetNode()->GetTransform().Rotation().EulerAngles().y_, targetRotation.EulerAngles().y_);
            float yTorqueCorrection = y_Axis.UpdateAsPID(yAngleError, timeStep);

            float zAngleError =
                DeltaAngle(GetNode()->GetTransform().Rotation().EulerAngles().z_, targetRotation.EulerAngles().z_);
            float zTorqueCorrection = z_Axis.UpdateAsPID(zAngleError, timeStep);

            rigidBody->ApplyTorque((xTorqueCorrection * Vector3::RIGHT) + (yTorqueCorrection * Vector3::UP) +
                                   (zTorqueCorrection * Vector3::FORWARD));
            rigidBody->ApplyForce((Vector3::FORWARD) * thrust * timeStep);
        }
    }

    void Update(float timeStep) override
    {
        x_Axis.P = _xAxisP;
        x_Axis.I = _xAxisI;
        x_Axis.D = _xAxisD;

        y_Axis.P = _yAxisP;
        y_Axis.I = _yAxisI;
        y_Axis.D = _yAxisD;

        z_Axis.P = _zAxisP;
        z_Axis.I = _zAxisI;
        z_Axis.D = _zAxisD;
    }

    bool animate_ = false;

    FPIDController x_Axis;
    FPIDController y_Axis;
    FPIDController z_Axis;

    float _xAxisP = .5f, _xAxisI = .3f, _xAxisD = .2f;
    float _yAxisP = .5f, _yAxisI = .3f, _yAxisD = .2f;
    float _zAxisP = .5f, _zAxisI = .3f, _zAxisD = .2f;

    float thrust = 0.f;
};
} // namespace Urho3D
