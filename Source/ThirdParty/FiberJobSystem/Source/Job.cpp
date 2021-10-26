#include "../Include/FJS/Job.h"
#include "../Include/FJS/Counter.h"

void FJS::JobInfo::Execute()
{
	if (!IsNull())
		GetDelegate()->Call();

	if (m_counter)
		m_counter->Decrement();
}
