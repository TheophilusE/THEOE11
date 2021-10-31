
#include "../Engine/Event.h"

namespace Urho3D
{
Event::Event(const Type& type, const Variant& data)
    : m_Type(type)
    , m_Data(data)
{
}

} // namespace Urho3D
