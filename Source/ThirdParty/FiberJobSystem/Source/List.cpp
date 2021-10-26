#include "../Include/FJS/List.h"
#include "../Include/FJS/Manager.h"

FJS::List::List(FJS::Manager* mgr, JobPriority defaultPriority) :
	m_manager(mgr),
	m_defaultPriority(defaultPriority),
	m_counter(mgr)
{}

FJS::List::~List()
{}

void FJS::List::Add(JobPriority prio, JobInfo job)
{
	job.SetCounter(&m_counter);

	m_manager->ScheduleJob(prio, job);
}

FJS::List& FJS::List::operator+=(const JobInfo& job)
{
	Add(m_defaultPriority, job);
	return *this;
}

void FJS::List::Wait(uint32_t targetValue)
{
	m_manager->WaitForCounter(&m_counter, targetValue);
}
