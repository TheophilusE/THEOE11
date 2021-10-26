#include "../Include/FJS/Fiber.h"
#include "../Include/FJS/Common.h"
#ifdef _WIN32
#include <Windows.h>
#endif

// TODO: Add exceptions for invalid stuff?

static void LaunchFiber(FJS::Fiber* fiber)
{
	auto callback = fiber->GetCallback();
	if (callback == nullptr)
		throw FJS::Exception("LaunchFiber: callback is nullptr");

	callback(fiber);
}

FJS::Fiber::Fiber()
{
#ifdef _WIN32
	m_fiber = CreateFiber(0, (LPFIBER_START_ROUTINE)LaunchFiber, this);
	m_thread_fiber = false;
#endif
}

FJS::Fiber::~Fiber()
{
#ifdef _WIN32
	if (m_fiber && !m_thread_fiber)
		DeleteFiber(m_fiber);
#endif
}

void FJS::Fiber::FromCurrentThread()
{
#ifdef _WIN32
	if (m_fiber && !m_thread_fiber)
		DeleteFiber(m_fiber);

	m_fiber = ConvertThreadToFiber(nullptr);
	m_thread_fiber = true;
#endif
}

void FJS::Fiber::SetCallback(Callback_t cb)
{
	if (cb == nullptr)
		throw FJS::Exception("callback cannot be nullptr");

	m_callback = cb;
}

void FJS::Fiber::SwitchTo(FJS::Fiber* fiber, void* userdata)
{
	if (fiber == nullptr || fiber->m_fiber == nullptr)
		throw FJS::Exception("Invalid fiber (nullptr or invalid)");

	fiber->m_userdata = userdata;
	fiber->m_return_fiber = this;

	SwitchToFiber(fiber->m_fiber);
}

void FJS::Fiber::SwitchBack()
{
	if (m_return_fiber && m_return_fiber->m_fiber)
		SwitchToFiber(m_return_fiber->m_fiber);
	else
		throw FJS::Exception("Unable to switch back from Fiber (none or invalid return fiber)");
}
