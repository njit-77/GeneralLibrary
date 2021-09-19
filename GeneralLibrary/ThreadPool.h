#pragma once

#include "pch.h"

#include <vector>
#include <queue>
#include <atomic>
#include <future>
#include <stdexcept>

#define THREADPOOL_MAX_NUM 16

class ThreadPool
{
	/// ��������
	using Task = std::function<void()>;

	/// �̳߳�
	std::vector<std::thread> _pool;

	/// �������
	std::queue<Task> _tasks;

	/// ͬ��
	std::mutex _lock;

	/// ��������
	std::condition_variable _task_cv;

	/// �̳߳��Ƿ�ִ��
	std::atomic<bool> _run{ true };

	/// �����߳�����
	std::atomic<int>  _idlThrNum{ 0 };

public:
	inline ThreadPool(unsigned short size = 4) { addThread(size); }
	inline ~ThreadPool()
	{
		_run = false;
		/// ���������߳�ִ��
		_task_cv.notify_all();
		for (std::thread& thread : _pool)
		{
			/// ���̡߳���������
			/// thread.detach();
			if (thread.joinable())
				/// �ȴ���������� ǰ�᣺�߳�һ����ִ����
				thread.join();
		}
	}

public:
	/// �ύһ������
	/// ����.get()��ȡ����ֵ��ȴ�����ִ����,��ȡ����ֵ
	/// �����ַ�������ʵ�ֵ������Ա��
	/// һ����ʹ��   bind�� .commit(std::bind(&Dog::sayHello, &dog));
	/// һ������   mem_fn�� .commit(std::mem_fn(&Dog::sayHello), this)
	template<class F, class... Args>
	auto commit(F&& f, Args&&... args) ->std::future<decltype(f(args...))>
	{
		if (!_run)
			throw std::runtime_error("commit on ThreadPool is stopped.");

		/// typename std::result_of<F(Args...)>::type, ���� f �ķ���ֵ����
		using RetType = decltype(f(args...));

		/// �Ѻ�����ڼ�����,���(��)
		auto task = std::make_shared<std::packaged_task<RetType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

		std::future<RetType> future = task->get_future();
		{
			/// ������񵽶���
			/// �Ե�ǰ���������lock_guard��mutex��stack��װ�࣬�����ʱ��lock()��������ʱ��unlock()
			std::lock_guard<std::mutex> lock{ _lock };
			_tasks.emplace([task]() {(*task)(); });
		}

		/// ����һ���߳�ִ��
		_task_cv.notify_one();

		return future;
	}

	/// �����߳�����
	int idlCount() { return _idlThrNum; }

	/// �߳�����
	size_t thrCount() { return _pool.size(); }

private:
	/// ���ָ���������߳�
	void addThread(unsigned short size)
	{
		for (; _pool.size() < THREADPOOL_MAX_NUM && size > 0; --size)
		{
			/// �����߳�����,�������� Ԥ�������� THREADPOOL_MAX_NUM
			_pool.emplace_back([this] { /// �����̺߳���
				while (_run)
				{
					/// ��ȡһ����ִ�е� task
					Task task;
					{
						/// unique_lock ��� lock_guard �ĺô��ǣ�������ʱ unlock() �� lock()
						std::unique_lock<std::mutex> lock{ _lock };
						_task_cv.wait(lock, [this] {
							return !_run || !_tasks.empty();
						});
						/// wait ֱ���� task
						if (!_run && _tasks.empty())
							return;
						/// ���Ƚ��ȳ��Ӷ���ȡһ�� task
						task = move(_tasks.front());
						_tasks.pop();
					}
					_idlThrNum--;
					/// ִ������
					task();
					_idlThrNum++;
				}
			});
			_idlThrNum++;
		}
	}

public:
	static ThreadPool& GetInstance()
	{
		static ThreadPool m_instance{ 10 };
		return m_instance;
	}
};

/// ���Դ���
//#ifdef DEBUG_LOG
//	LogTrace("%s In.", __func__);
//#endif
//	std::vector<std::future<std::string>> results1;
//	std::vector<std::future<void>> results2;
//	for (size_t i = 0; i < 15; i++)
//	{
//		results1.push_back(ThreadPool::GetInstance().commit([]()->std::string
//		{
//#ifdef DEBUG_LOG
//			LogTrace("hello1 %s threadid = %d.", __func__, std::this_thread::get_id());
//#endif
//			return "hello world";
//		}));
//	}
//	for (size_t i = 0; i < 15; i++)
//	{
//		results2.push_back(ThreadPool::GetInstance().commit([]
//		{
//#ifdef DEBUG_LOG
//			LogTrace("hello2 %s threadid = %d.", __func__, std::this_thread::get_id());
//#endif
//		}));
//	}
//	for (size_t i = 0; i < 15; i++)
//	{
//		results1[i].get();
//		results2[i].get();
//	}
//#ifdef DEBUG_LOG
//	LogTrace("%s Out.", __func__);
//#endif
