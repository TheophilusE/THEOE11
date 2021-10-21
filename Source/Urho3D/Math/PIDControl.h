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
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/LogicComponent.h>

namespace Urho3D
{
/**
 * PID Controller
 * Error: where you are vs where you want to be
 * Derivative: how fast you are approaching, dampening
 * Integral: alignment error
 */
struct FPIDController
{
public:
    // Proportional gain
    float P = 0.0f;

    // Integral gain
    float I = 0.0f;

    // Derivative gain
    float D = 0.0f;

    // Max output (as absolute value)
    float MaxOutAbs = 0.0f;

    // Default constructor (no initialization)
    FPIDController() {}

    // Constructor with initial value for each component
    FPIDController(float InP, float InI, float InD, float InMaxOutAbs);

    // Update type function pointer variable
    typedef float (FPIDController::*UpdateTypeFunctionPtr)(const float, const float);

    // Update type function ptr
    UpdateTypeFunctionPtr UpdateFunctionPtr;

    //  Set PID values, reset error values, bind update function ptr
    void Init(float InP, float InI, float InD, float InMaxOutAbs, bool bClearErrors = true);

    // Reset error values, bind update function ptr
    void Init(bool bClearErrors = true);

    // Update the PID loop
    FORCEINLINE float Update(const float InError, const float InDeltaTime);

    // Update as a PID controller
    FORCEINLINE float UpdateAsPID(const float InError, const float InDeltaTime);

    // Update as a P controller
    FORCEINLINE float UpdateAsP(const float InError, const float InDeltaTime = 0);

    // Update as a PD controller
    FORCEINLINE float UpdateAsPD(const float InError, const float InDeltaTime);

    // Update as a PI controller
    FORCEINLINE float UpdateAsPI(const float InError, const float InDeltaTime);

private:
    // Previous step error value
    float PrevErr;

    // Integral error
    float IErr;
};

/**
 * PID Controller for FVector
 * Error: where you are vs where you want to be
 * Derivative: how fast you are approaching, dampening
 * Integral: alignment error
 */
struct FPIDController3D
{

public:
    // Proportional gain
    float P = 0.f;

    // Integral gain
    float I = 0.f;

    // Derivative gain
    float D = 0.f;

    // Max output (as absolute value)
    float MaxOutAbs = 0.f;

    // Default constructor (no initialization)
    FPIDController3D() {}

    // Constructor with initial value for each component
    FPIDController3D(float InP, float InI, float InD, float InMaxOutAbs);

    // Update type function pointer variable
    typedef Vector3 (FPIDController3D::*UpdateTypeFunctionPtr)(const Vector3, const float);

    // Update type function ptr
    UpdateTypeFunctionPtr UpdateFunctionPtr;

    // Set PID values, reset error values, bind update function ptr
    void Init(float InP, float InI, float InD, float InMaxOutAbs, bool bClearErrors = true);

    // Reset error values, bind update function ptr
    void Init(bool bClearErrors = true);

    // Update the PID loop
    FORCEINLINE Vector3 Update(const Vector3 InError, const float InDeltaTime);

    // Update as a PID controller
    FORCEINLINE Vector3 UpdateAsPID(const Vector3 InError, const float InDeltaTime);

    // Update as a P controller
    FORCEINLINE Vector3 UpdateAsP(const Vector3 InError, const float InDeltaTime = 0.f);

    // Update as a PD controller
    FORCEINLINE Vector3 UpdateAsPD(const Vector3 InError, const float InDeltaTime);

    // Update as a PI controller
    FORCEINLINE Vector3 UpdateAsPI(const Vector3 InError, const float InDeltaTime);

private:
    // Previous step error value
    Vector3 PrevErr;

    // Integral error
    Vector3 IErr;
};

}
