#ifndef _JOBQUEUE_HPP_
#define _JOBQUEUE_HPP_

#include <boost/thread/condition.hpp>
#include <list>
namespace tlclass
{
	/**
	* 工作队列模板类，现在实际用处是一个上了线程锁的双端队列，两个重要的操作:向队列中添加新的job，和从队列中获取最早的job.
	*/
	template <typename Job>
	class jobqueue
	{
	public:
		jobqueue():_size(0)
  		{}

		virtual ~jobqueue()
		{}

		void submitjob(const Job& x)
		{
			boost::mutex::scoped_lock lock(_mutex);
			_list.push_back(x);
			_size++;
			_worktobedone.notify_all();
		}

		Job getjob();

		// 获取队列中剩余job的数量
		// 用到了list：：size接口，job比较多时速度较慢
		// 谨慎使用
		size_t SLOW_GetJobNum() const{boost::mutex::scoped_lock lock(_mutex); return _size;}

		// 获取队列中的job，若队列已空，则返回false
		bool GetJobInNoneBlockMode(Job &job); 
		// void notify_all()
		// {
		// 	_exitwait = true;
		// 	_worktobedone.notify_all();
		// }
	
	private:
		//bool _exitwait;
		std::list<Job> _list;
		mutable boost::mutex _mutex;
		boost::condition _worktobedone;
		size_t _size;
	};

	template <typename Job>
	bool jobqueue<Job>::GetJobInNoneBlockMode( Job &job )
	{
		boost::mutex::scoped_lock lock(_mutex);
		if(_list.empty())
			return false;
		job = _list.front();
		_list.pop_front();
		_size--;
		return true;
	}

	template<typename Job>
	Job jobqueue<Job>::getjob()
	{
		boost::mutex::scoped_lock lock(_mutex);
		while(_list.empty())
			_worktobedone.wait(lock);

		Job tmp = _list.front();
		_list.pop_front();
		_size--;
		return tmp;
	}


};

#endif	//_JOBQUEUE_HPP_

