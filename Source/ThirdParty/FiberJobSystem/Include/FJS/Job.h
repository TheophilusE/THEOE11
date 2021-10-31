#pragma once
#include "Detail/mpmc_queue.h"
#include "Detail/delegate.h"
#include "Counter.h"

namespace FJS
{
	class Counter;

	class JobInfo
	{
		// Delegate Buffer
		static constexpr size_t BufferSize = sizeof(void*) * (8);

		char m_buffer[BufferSize] = { 0 };
		inline Detail::base_delegate* GetDelegate() { return reinterpret_cast<Detail::base_delegate*>(m_buffer); };
		inline bool IsNull() const { return *(void**)m_buffer == nullptr; };

		void Reset()
		{
			if (!IsNull())
			{
				GetDelegate()->~base_delegate();
				*(void**)m_buffer = nullptr;
			}
		}

		// Store
		template <typename delegate_t, typename... Args>
		void StoreJobInfo(Args... args)
		{
			Detail::delegate_size_checker<sizeof(delegate_t), BufferSize>::check();
			new(m_buffer) delegate_t(args...);
		}

		template <class TClass, typename... Args>
		void StoreJobInfo(TClass* inst, Args... args)
		{
			using Ret = Detail::function_traits<TClass>::return_type;
			StoreJobInfo<typename Detail::delegate_member<TClass, Ret, Args...>>(&TClass::operator(), inst, args...);
		}

		// Counter
		Counter* m_counter = nullptr;

	public:
		JobInfo() = default;

		// Callable class (Lambda / function with operator())
		template <typename TCallable, typename... Args>
		JobInfo(Counter* ctr, TCallable callable, Args... args) :
			m_counter(ctr)
		{
			Reset();
			Detail::function_checker<TCallable, Args...>::check();

			StoreJobInfo<typename Detail::delegate_callable<TCallable, Args...>>(callable, args...);
		}

		// Function
		template <typename Ret, typename... Args>
		JobInfo(Counter* ctr, Ret(*function)(Args...), Args... args) :
			m_counter(ctr)
		{
			Reset();
			Detail::function_checker<decltype(function), Args...>::check();

			StoreJobInfo<typename Detail::delegate_callable<decltype(function), Args...>>(function, args...);
		}

		// Pointer to a callable class (operator())
		template <class TCallable, typename... Args>
		JobInfo(Counter* ctr, TCallable* callable, Args... args) :
			m_counter(ctr)
		{
			Reset();

			Detail::function_checker<TCallable, Args...>::check();
			StoreJobInfo(callable, args...);
		}

		// Member Function
		template <class TClass, typename Ret, typename... Args>
		JobInfo(Counter* ctr, Ret(TClass::* callable)(Args...), TClass* inst, Args... args) :
			m_counter(ctr)
		{
			Reset();
			Detail::function_checker<decltype(callable), TClass*, Args...>::check();

			StoreJobInfo<typename Detail::delegate_member<TClass, Ret, Args...>>(callable, inst, args...);
		}

		// Constructor without Counter
		template <typename TCallable, typename... Args>
		JobInfo(TCallable callable, Args... args) :
			JobInfo((Counter*)nullptr, callable, args...)
		{};

		template <typename Ret, typename... Args>
		JobInfo(Ret(*function)(Args...), Args... args) :
			JobInfo((Counter*)nullptr, function, args...)
		{};

		template <class TCallable, typename... Args>
		JobInfo(TCallable* callable, Args... args) :
			JobInfo((Counter*)nullptr, callable, args...)
		{};

		template <class TClass, typename Ret, typename... Args>
		JobInfo(Ret(TClass::* callable)(Args...), TClass* inst, Args... args) :
			JobInfo((Counter*)nullptr, callable, inst, args...)
		{};

		~JobInfo()
		{
			Reset();
		}

		// Counter
		inline void SetCounter(Counter* ctr)
		{
			m_counter = ctr;
		}

		inline Counter* GetCounter() const
		{
			return m_counter;
		}

		// Execute Job
		void Execute();

		// Assign Operator
		JobInfo& operator=(const JobInfo& other)
		{
			memcpy(m_buffer, other.m_buffer, BufferSize);
			m_counter = other.m_counter;

			return *this;
		}
	};

	enum class JobPriority : uint8_t
	{
		High,		// Jobs are executed ASAP
		Normal,
		Low
	};

	namespace Detail
	{
		// avoid confusion between FJS::Queue and FJS::JobQueue
		using JobQueue = Detail::mpmc_queue<JobInfo>;
	}
}