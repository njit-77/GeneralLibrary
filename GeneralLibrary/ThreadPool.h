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
	/// 定义类型
	using Task = std::function<void()>;

	/// 线程池
	std::vector<std::thread> _pool;

	/// 任务队列
	std::queue<Task> _tasks;

	/// 同步
	std::mutex _lock;

	/// 条件阻塞
	std::condition_variable _task_cv;

	/// 线程池是否执行
	std::atomic<bool> _run{ true };

	/// 空闲线程数量
	std::atomic<int>  _idlThrNum{ 0 };

public:
	inline ThreadPool(unsigned short size = 4) { addThread(size); }
	inline ~ThreadPool()
	{
		_run = false;
		/// 唤醒所有线程执行
		_task_cv.notify_all();
		for (std::thread& thread : _pool)
		{
			/// 让线程“自生自灭”
			/// thread.detach();
			if (thread.joinable())
				/// 等待任务结束， 前提：线程一定会执行完
				thread.join();
		}
	}

public:
	/// 提交一个任务
	/// 调用.get()获取返回值会等待任务执行完,获取返回值
	/// 有两种方法可以实现调用类成员，
	/// 一种是使用   bind： .commit(std::bind(&Dog::sayHello, &dog));
	/// 一种是用   mem_fn： .commit(std::mem_fn(&Dog::sayHello), this)
	template<class F, class... Args>
	auto commit(F&& f, Args&&... args) ->std::future<decltype(f(args...))>
	{
		if (!_run)
			throw std::runtime_error("commit on ThreadPool is stopped.");

		/// typename std::result_of<F(Args...)>::type, 函数 f 的返回值类型
		using RetType = decltype(f(args...));

		/// 把函数入口及参数,打包(绑定)
		auto task = std::make_shared<std::packaged_task<RetType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

		std::future<RetType> future = task->get_future();
		{
			/// 添加任务到队列
			/// 对当前块的语句加锁lock_guard是mutex的stack封装类，构造的时候lock()，析构的时候unlock()
			std::lock_guard<std::mutex> lock{ _lock };
			_tasks.emplace([task]() {(*task)(); });
		}

		/// 唤醒一个线程执行
		_task_cv.notify_one();

		return future;
	}

	/// 空闲线程数量
	int idlCount() { return _idlThrNum; }

	/// 线程数量
	size_t thrCount() { return _pool.size(); }

private:
	/// 添加指定数量的线程
	void addThread(unsigned short size)
	{
		for (; _pool.size() < THREADPOOL_MAX_NUM && size > 0; --size)
		{
			/// 增加线程数量,但不超过 预定义数量 THREADPOOL_MAX_NUM
			_pool.emplace_back([this] { /// 工作线程函数
				while (_run)
				{
					/// 获取一个待执行的 task
					Task task;
					{
						/// unique_lock 相比 lock_guard 的好处是：可以随时 unlock() 和 lock()
						std::unique_lock<std::mutex> lock{ _lock };
						_task_cv.wait(lock, [this] {
							return !_run || !_tasks.empty();
						});
						/// wait 直到有 task
						if (!_run && _tasks.empty())
							return;
						/// 按先进先出从队列取一个 task
						task = move(_tasks.front());
						_tasks.pop();
					}
					_idlThrNum--;
					/// 执行任务
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

/// 测试代码
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
