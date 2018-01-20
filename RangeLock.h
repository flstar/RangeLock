#ifndef __RANGE_LOCK_H__
#define __RANGE_LOCK_H__

#include <memory>
#include <queue>
#include <mutex>

template <typename T>
class RangeLock
{
public:
	template <typename T2>
	class Lock
	{
		friend class RangeLock<T2>;
	public:
		Lock(const T2 &start, const T2 &end)
		{
			start_ = start;
			end_ = end;
		}
		virtual ~Lock() {}
	private:
		T2 start_;
		T2 end_;
		std::condition_variable cv_;

	public:
		/** @brief this lock blocks the parameter lock */
		bool block(const Lock &lock)
		{
			if (lock.end_ < start_ || end_ < lock.start_) return false;
			else return true;
		}
	};

public:
	RangeLock() {}
	virtual ~RangeLock() {}

	RangeLock(const & RangeLock) = delete;
	RangeLock & operator = (const RangeLock &) = delete;

	typedef std::shared_ptr<Lock<T>> LockHandler;
private:
	std::mutex m_;
	std::deque<LockHandler> lockedq_;
	std::deque<LockHandler> waitingq_;

public:
	LockHandler lock(const T &start, const T &end)
	{
		LockHandler handler(new Lock<T>(start, end));
		
		std::unique_lock<std::mutex> guard(m_);
		waitingq_.push_back	(handler);

		while (true) {
			restart:
			// check all locks
			for (auto iter = lockedq_.begin(); iter != lockedq_.end(); iter++) {
				if ((*iter)->block(*handler)) {
					(*iter)->cv_.wait(guard);
					goto restart;
				}
			}
			// check all waiting locks before current one
			for (auto iter = waitingq_.begin(); iter != waitingq_.end(); iter++) {
				if ((*iter) == handler) {
					waitingq_.erase(iter);
					lockedq_.push_back(*iter);
					return handler;
				}
				else if ((*iter)->block(*handler)) {
					(*iter)->cv_.wait(guard);
					goto restart;
				}
			}
		}
	}

	void unlock(LockHandler handler)
	{
		std::unique_lock<std::mutex> guard(m_);
		for (auto iter = lockedq_.begin(); iter != lockedq_.end(); iter++) {
			if (handler == (*iter)) {
				iter = lockedq_.erase(iter);
				break;
			}
		}
		handler->cv_.notify_all();
	}
};

#endif
