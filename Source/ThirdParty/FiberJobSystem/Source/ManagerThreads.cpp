#include "../Include/FJS/Manager.h"
#include "../Include/FJS/Thread.h"
#ifdef _WIN32
#include <Windows.h>
#endif

uint8_t FJS::Manager::GetCurrentThreadIndex() const
{
#ifdef _WIN32
	uint32_t idx = GetCurrentThreadId();
	for (uint8_t i = 0; i < m_numThreads; i++)
	{
		if (m_threads[i].GetID() == idx)
			return i;
	}
#endif

	return UINT8_MAX;
}

FJS::Thread* FJS::Manager::GetCurrentThread() const
{
#ifdef _WIN32
	uint32_t idx = GetCurrentThreadId();
	for (uint8_t i = 0; i < m_numThreads; i++)
	{
		if (m_threads[i].GetID() == idx)
			return &m_threads[i];
	}
#endif

	return nullptr;
}

FJS::TLS* FJS::Manager::GetCurrentTLS() const
{
#ifdef _WIN32
	uint32_t idx = GetCurrentThreadId();
	for (uint8_t i = 0; i < m_numThreads; i++)
	{
		if (m_threads[i].GetID() == idx)
			return m_threads[i].GetTLS();
	}
#endif

	return nullptr;
}
