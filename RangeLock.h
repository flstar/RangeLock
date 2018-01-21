#ifndef __RANGE_LOCK_H__
#define __RANGE_LOCK_H__

#include <memory>
#include <queue>
#include <mutex>


template <typename T>
class RangeLock
{
public:
	class Lock
	{
		friend class RangeLock;
	public:
		Lock(const T &start, const T &end, bool wlocked = true)
		{
			start_ = start;
			end_ = end;
			wlocked_ = wlocked;
		}
		virtual ~Lock() {}
	private:
		T start_;
		T end_;
		bool wlocked_;
		std::condition_variable cv_;

	private:
		/** @brief this lock blocks the parameter lock */
		bool block(const Lock &lock)
		{
			if (!wlocked_ && !lock.wlocked_) {
				// if both are readonly, they won't block each other
				return false;
			}
			else if (lock.end_ < start_ || end_ < lock.start_) {
				// if they don't overlap with each other, they won't blocked each other
				return false;
			}
			else {
				return true;
			}
		}

	};
	typedef std::shared_ptr<Lock> LockHandler;

public:
	RangeLock() {}
	virtual ~RangeLock() {}

	RangeLock(const & RangeLock) = delete;
	RangeLock & operator = (const RangeLock &) = delete;

private:
	std::mutex m_;
	std::deque<LockHandler> lockedq_;
	std::deque<LockHandler> waitingq_;

public:
	LockHandler lock(const T &start, const T &end, bool wlocked = true)
	{
		LockHandler handler(new Lock(start, end, wlocked));
		
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

