#ifndef __RANGE_LOCK_H__
#define __RANGE_LOCK_H__

#include <memory>
#include <unordered_map>
#include <queue>
#include <mutex>


template <typename T>
class RangeLock
{
public:
	template <typename T2>
	class Range
	{
		friend class RangeLock<T2>;
	public:
		Range(const T2 &start, const T2 &end)
		{
			start_ = start;
			end_ = end;
		}
		virtual ~Range() {}
	private:
		T2 start_;
		T2 end_;
		std::condition_variable cv_;
		
	public:
		bool overlap(const Range &r)
		{
			if (r.end_ < start_ || end_ < r.start_) return false;
			else return true;
		}
	};
		
public:
	RangeLock() {}
	virtual ~RangeLock() {}

	RangeLock(const & RangeLock) = delete;
	RangeLock & operator = (const RangeLock &) = delete;

	typedef std::shared_ptr<Range<T>> RangeHandler;
private:
	std::mutex m_;
	std::queue<RangeHandler> lockedq_;
	std::queue<RangeHandler> waitingq_;

public:
	RangeHandler lock(const T &start, const T &end)
	{
		RangeHandler range(new Range<T>(start, end));
		
		std::lock_guard<std::mutex> guard(m_);
		waitingq_.push(range);

		while (true) {
			// check all locked ranges

			// check all waiting ranges ahead

		}
	}
	
	void unlock(RangeHandler range_id);

};

#endif
