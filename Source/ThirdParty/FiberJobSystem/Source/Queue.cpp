#include "../Include/FJS/Queue.h"
#include "../Include/FJS/Manager.h"

FJS::Queue::Queue(FJS::Manager* mgr, JobPriority defaultPriority) :
	m_manager(mgr),
	m_defaultPriority(defaultPriority),
	m_counter(mgr)
{}

FJS::Queue::~Queue()
{}

void FJS::Queue::Add(JobPriority prio, JobInfo job)
{
	job.SetCounter(&m_counter);
	m_queue.emplace_back(prio, job);
}

FJS::Queue& FJS::Queue::operator+=(const JobInfo& job)
{
	Add(m_defaultPriority, job);
	return *this;
}

bool FJS::Queue::Step()
{
	if (m_queue.empty())
		return false;

	const auto& job = m_queue.front();
	m_manager->ScheduleJob(job.first, job.second);
	m_manager->WaitForCounter(&m_counter);

	m_queue.erase(m_queue.begin());
	return true;
}

void FJS::Queue::Execute()
{
	while (Step())
		;
}
