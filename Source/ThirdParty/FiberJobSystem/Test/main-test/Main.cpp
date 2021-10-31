#include <FJS/Manager.h>
#include <FJS/Counter.h>
#include <FJS/List.h>
#include <FJS/Queue.h>
#include <iostream>
#include <Windows.h>

void test_job_1(int* x)
{
	std::cout << "test_job_1 with " << *x << std::endl;
	(*x)++;
}

void just_count()
{
	for (int i = 0; i < 1000; ++i)
		std::cout << "Num: " << i << std::endl;
}

struct test_job_2
{
	void Execute(int* x)
	{
		std::cout << "test_job_2::Execute with " << *x << std::endl;
		(*x)++;
	}

	void operator()(int* x)
	{
		std::cout << "test_job_2::operator() with " << *x << std::endl;
		(*x)++;
	}
};

void main_test(FJS::Manager* mgr)
{
	int count = 1;

	// 1: Function
	mgr->WaitForSingle(FJS::JobPriority::Normal, test_job_1, &count);

	/// 1a: Function
	mgr->ScheduleJob(FJS::JobPriority::Low, just_count);

	// 2: Lambda
	mgr->WaitForSingle(FJS::JobPriority::Normal, [&count]() {
		std::cout << "lambda with " << count << std::endl;
		count++;
	});

	// 2a: Lambda
	mgr->ScheduleJob(FJS::JobPriority::Normal, []() {
		for (int i = 0; i < 1000; ++i)
			std::cout << "lambda with " << i << std::endl;
	});

	// 3: Member Function
	test_job_2 tj2_inst;
	mgr->WaitForSingle(FJS::JobPriority::Normal, &test_job_2::Execute, &tj2_inst, &count);

	// 3: Class operator()
	mgr->WaitForSingle(FJS::JobPriority::Normal, &tj2_inst, &count);

	// Counter
	FJS::Counter counter(mgr);

	// It's also possible to create a JobInfo yourself
	// First argument can be a Counter
	FJS::JobInfo test_job(&counter, test_job_1, &count);
	mgr->ScheduleJob(FJS::JobPriority::Normal, test_job);
	mgr->WaitForCounter(&counter);

	// List / Queues
	FJS::List list(mgr);
	list.Add(FJS::JobPriority::Normal, test_job_1, &count);
	//list += test_job; This would be unsafe, Jobs might execute in parallel

	list.Wait();

	FJS::Queue queue(mgr, FJS::JobPriority::High); // default Priority is high
	queue.Add(test_job_1, &count);
	queue += test_job; // Safe, Jobs are executed consecutively

	queue.Execute();
}

int main()
{
	// Setup Job Manager
	FJS::ManagerOptions managerOptions;
	managerOptions.NumFibers = managerOptions.NumThreads * 10;
	managerOptions.ThreadAffinity = true;
	
	managerOptions.HighPriorityQueueSize = 128;
	managerOptions.NormalPriorityQueueSize = 256;
	managerOptions.LowPriorityQueueSize = 256;

	managerOptions.ShutdownAfterMainCallback = true;

	// Manager
	FJS::Manager manager(managerOptions);

	if (manager.Run(main_test) != FJS::Manager::ReturnCode::Succes)
		std::cout << "oh no" << std::endl;
	else
		std::cout << "done" << std::endl;

	// Don't close
	getchar();
	return 0;
}