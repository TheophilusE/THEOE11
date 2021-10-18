
#include "../Engine/EventManager.h"

namespace Urho3D
{
EventManager* EventManager::GetSingleton()
{
    static EventManager singleton;
    return &singleton;
}

void EventManager::Defer(eastl::function<void()> function) { m_DeferList.push_back(function); }

void EventManager::AddBinder(EventBinderBase* binder)
{
    if (m_EventBinders.find(binder) == m_EventBinders.end())
    {
        m_EventBinders[binder] = true;
    }
}

void EventManager::RemoveBinder(EventBinderBase* binder)
{
    if (m_EventBinders.find(binder) != m_EventBinders.end())
    {
        m_EventBinders.erase(binder);
    }
}

Variant EventManager::ReturnCall(const Event& event) const
{
    URHO3D_PROFILE("Event Manager Return Call");
    for (auto& [binder, value] : m_EventBinders)
    {
        if (binder->HasBinding(event.GetType()))
        {
            return binder->Handle(event);
        }
    }
    return false;
}

void EventManager::Call(const Event& event) const
{
    URHO3D_PROFILE("Event Manager Call");
    for (auto& [binder, value] : m_EventBinders)
    {
        if (binder->HasBinding(event.GetType()))
        {
            binder->Handle(event);
        }
    }
}

Variant EventManager::ReturnCall(const Event::Type& eventType, const Variant& data) const
{
    Event event(eventType, data);
    return ReturnCall(event);
}

void EventManager::Call(const Event::Type& eventType, const Variant& data) const
{
    Event event(eventType, data);
    Call(event);
}

void EventManager::DeferredCall(eastl::shared_ptr<Event> event) { m_DeferredCalls.push_back(event); }

void EventManager::DeferredCall(const Event::Type& eventType, const Variant& data)
{
    eastl::shared_ptr<Event> event(new Event(eventType, data));
    DeferredCall(event);
}

void EventManager::DispatchDeferred()
{
    URHO3D_PROFILE("Dispatch Deferred Events");
    for (auto& function : m_DeferList)
    {
        function();
    }
    m_DeferList.clear();

    for (auto& deferredEvent : m_DeferredCalls)
    {
        Call(*deferredEvent);
    }
    m_DeferredCalls.clear();
}
} // namespace Urho3D
