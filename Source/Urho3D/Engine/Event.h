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

#include "../Core/Object.h"

#define DEFINE_EVENT(eventName, ...) const static inline Event::Type eventName = #eventName

namespace Urho3D
{
class URHO3D_API Event
{
public:
    /// String defining the type of event
    typedef ea::string Type;

private:
    Type m_Type;
    Variant m_Data;

public:
    Event(const Type& type, const Variant& data);
    Event(Event&) = delete;
    ~Event() = default;

    /// Returns Event Type as String
    const Type& GetType() const { return m_Type; };
    /// Returns the payload data sent with an event. Extract typed data after getting the data.
    const Variant& GetData() const { return m_Data; }
};
} // namespace Urho3D
