#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/UI/Text.h>
#include <EASTL/list.h>

using namespace Urho3D;

struct NotificationData
{
    eastl::string message;
    Color color;
};

class Notifications : public Object
{
    URHO3D_OBJECT(Notifications, Object);

public:
    Notifications(Context* context);

    virtual ~Notifications();

private:
    virtual void Init();

    /**
     * Subscribe to notification events
     */
    void SubscribeToEvents();

    /**
     * Handle ShowNotification event
     */
    void HandleNewNotification(StringHash eventType, VariantMap& eventData);

    void CreateNewNotification(NotificationData data);

    /**
     * Handle message displaying and animations
     */
    void HandleUpdate(StringHash eventType, VariantMap& eventData);

    /**
     * Handle game end event
     */
    void HandleGameEnd(StringHash eventType, VariantMap& eventData);

    /**
     * List of all active messages
     */
    eastl::vector<SharedPtr<UIElement>> messages_;
    SharedPtr<ObjectAnimation> notificationAnimation_;
    SharedPtr<ValueAnimation> positionAnimation_;
    SharedPtr<ValueAnimation> opacityAnimation_;
    eastl::list<NotificationData> messageQueue_;
    Timer timer_;
};
