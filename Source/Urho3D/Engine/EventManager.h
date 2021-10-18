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
#include "../Engine/Event.h"

namespace Urho3D
{
class URHO3D_API EventBinderBase
{
public:
    virtual bool HasBinding(const Event::Type& binding) const = 0;
    /// Call only if binding exists
    virtual Variant Handle(const Event& event) = 0;
};

template <class T> class URHO3D_API EventBinder : public EventBinderBase
{
    typedef eastl::function<Variant(const Event*)> EventFunction;

    eastl::unordered_map<Event::Type, EventFunction> m_Bindings;

public:
    EventBinder() { EventManager::GetSingleton()->AddBinder(this); }

    ~EventBinder() { EventManager::GetSingleton()->RemoveBinder(this); }

    /// Duplicate bindings will override the previous ones
    void Bind(const Event::Type& event, T* self, Variant (T::*eventFunction)(const Event*))
    {
        m_Bindings.emplace(event, [self, eventFunction](const Event* e) { return (self->*eventFunction)(e); });
    }

    void Bind(const Event::Type& event, EventFunction function) { m_Bindings.emplace(event, function); }

    void Unbind(const Event::Type& event) { m_Bindings.erase(event); }

    void UnbindAll() { m_Bindings.clear(); }

    bool HasBinding(const Event::Type& binding) const override { return m_Bindings.find(binding) != m_Bindings.end(); }

    Variant Handle(const Event& event) override { return m_Bindings.at(event.GetType())(&event); }
};

/// An Event dispatcher and registrar that also allows looking up registered events.
class URHO3D_API EventManager
{
    eastl::unordered_map<EventBinderBase*, bool> m_EventBinders;
    eastl::vector<eastl::function<void()>> m_DeferList;
    eastl::vector<eastl::shared_ptr<Event>> m_DeferredCalls;

public:
    static EventManager* GetSingleton();

    /// Defer a singular function till the end of the frame.
    void Defer(eastl::function<void()> function);

    /// Add an event binder which binds to several events per object. Does not need to be called externally.
    void AddBinder(EventBinderBase* binder);
    void RemoveBinder(EventBinderBase* binder);

    /// Publish an event. Returns the result of the first event handled.
    Variant ReturnCall(const Event& event) const;
    Variant ReturnCall(const Event::Type& eventType, const Variant& data = 0) const;

    void Call(const Event& event) const;
    void Call(const Event::Type& eventType, const Variant& data = 0) const;

    /// Publish an event that gets evaluated the end of the current frame.
    void DeferredCall(eastl::shared_ptr<Event> event);
    void DeferredCall(const Event::Type& eventType, const Variant& data = 0);

    /// Dispatch deferred events collected so far.
    void DispatchDeferred();

    const eastl::unordered_map<EventBinderBase*, bool>& GetBinders() const { return m_EventBinders; }
};
}
