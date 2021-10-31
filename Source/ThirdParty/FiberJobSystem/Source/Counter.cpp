#include "../Include/FJS/Common.h"
#include "../Include/FJS/Counter.h"
#include "../Include/FJS/Manager.h"
#include "../Include/FJS/TLS.h"

FJS::Counter::Counter(Manager* mgr) :
	m_manager(mgr)
{
	for (uint8_t i = 0; i < MAX_WAITING; i++)
		m_freeWaitingSlots[i].store(true);
}

FJS::Counter::Unit_t FJS::Counter::Increment(Unit_t by)
{
	Unit_t prev = m_counter.fetch_add(by);
	CheckWaitingFibers(prev + by);

	return prev;
}

FJS::Counter::Unit_t FJS::Counter::Decrement(Unit_t by)
{
	Unit_t prev = m_counter.fetch_sub(by);
	CheckWaitingFibers(prev - by);

	return prev;
}

FJS::Counter::Unit_t FJS::Counter::GetValue() const
{
	return m_counter.load(std::memory_order_seq_cst);
}

bool FJS::Counter::AddWaitingFiber(uint16_t fiberIndex, Unit_t targetValue, std::atomic_bool* fiberStored)
{
	for (uint8_t i = 0; i < MAX_WAITING; i++)
	{
		// Acquire Free Waiting Slot
		bool expected = true;
		if (!std::atomic_compare_exchange_strong_explicit(&m_freeWaitingSlots[i], &expected, false, std::memory_order_seq_cst, std::memory_order_relaxed))
			continue;

		// Setup Slot
		auto slot = &m_waiting[i];
		slot->FiberIndex = fiberIndex;
		slot->FiberStored = fiberStored;
		slot->TargetValue = targetValue;

		slot->InUse.store(false);

		// Check if we are done already
		Unit_t counter = m_counter.load(std::memory_order_relaxed);
		if (slot->InUse.load(std::memory_order_acquire))
			return false;

		if (slot->TargetValue == counter)
		{
			expected = false;
			if (!std::atomic_compare_exchange_strong_explicit(&slot->InUse, &expected, true, std::memory_order_seq_cst, std::memory_order_relaxed))
				return false;

			m_freeWaitingSlots[i].store(true, std::memory_order_release);
			return true;
		}

		return false;
	}

	// Waiting Slots are full
	throw FJS::Exception("Counter waiting slots are full!");
}

void FJS::Counter::CheckWaitingFibers(Unit_t value)
{
	for (size_t i = 0; i < MAX_WAITING; i++)
	{
		if (m_freeWaitingSlots[i].load(std::memory_order_acquire))
			continue;

		auto waitingSlot = &m_waiting[i];

		if (waitingSlot->InUse.load(std::memory_order_acquire))
			continue;

		if (waitingSlot->TargetValue == value)
		{
			bool expected = false;
			if (!std::atomic_compare_exchange_strong_explicit(&waitingSlot->InUse, &expected, true, std::memory_order_seq_cst, std::memory_order_relaxed))
				continue;

			m_manager->GetCurrentTLS()->ReadyFibers.emplace_back(waitingSlot->FiberIndex, waitingSlot->FiberStored);
			m_freeWaitingSlots[i].store(true, std::memory_order_release);
		}
	}
}
