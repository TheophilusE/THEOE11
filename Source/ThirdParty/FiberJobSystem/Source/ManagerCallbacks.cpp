#include "../Include/FJS/Manager.h"
#include "../Include/FJS/Thread.h"
#include "../Include/FJS/Fiber.h"
#include "../Include/FJS/Counter.h"

void FJS::Manager::ThreadCallback_Worker(FJS::Thread* thread)
{
	auto manager = reinterpret_cast<FJS::Manager*>(thread->GetUserdata());
	auto tls = thread->GetTLS();

	// Thread Affinity
	if (tls->SetAffinity)
		thread->SetAffinity(tls->ThreadIndex);

	// Setup Thread Fiber
	tls->ThreadFiber.FromCurrentThread();

	// Fiber
	tls->CurrentFiberIndex = manager->FindFreeFiber();

	auto fiber = &manager->m_fibers[tls->CurrentFiberIndex];
	tls->ThreadFiber.SwitchTo(fiber, manager);
}

void FJS::Manager::FiberCallback_Main(FJS::Fiber* fiber)
{
	auto manager = reinterpret_cast<FJS::Manager*>(fiber->GetUserdata());
	
	// Main
	manager->m_mainCallback(manager);

	// Shutdown after Main
	if (!manager->m_shutdownAfterMain)
	{
		// Switch to idle Fiber
		auto tls = manager->GetCurrentTLS();
		tls->CurrentFiberIndex = manager->FindFreeFiber();
		
		auto fiber = &manager->m_fibers[tls->CurrentFiberIndex];
		tls->ThreadFiber.SwitchTo(fiber, manager);
	}

	// Shutdown
	manager->Shutdown(false);

	// Switch back to Main Thread
	fiber->SwitchBack();
}

void FJS::Manager::FiberCallback_Worker(FJS::Fiber* fiber)
{
	auto manager = reinterpret_cast<FJS::Manager*>(fiber->GetUserdata());
	manager->CleanupPreviousFiber();

	JobInfo job;

	while (!manager->IsShuttingDown())
	{
		auto tls = manager->GetCurrentTLS();
		
		if (manager->GetNextJob(job, tls))
		{
			job.Execute();
			continue;
		}

		Thread::Sleep(1);
	}
	
	// Switch back to Thread
	fiber->SwitchBack();
}
