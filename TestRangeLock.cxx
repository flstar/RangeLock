#include <future>
#include <gtest/gtest.h>

#include "RangeLock.h"

using namespace std;

TEST(RangeLock, LockUnlock)
{
	RangeLock<int> locker;

	auto h = locker.lock(0, 1);
	locker.unlock(h);
}

TEST(RangeLock, SimpleLockBlocked)
{
	RangeLock<int> locker;
	int step = 0;

	auto f = async([&locker, &step]() {
		step = 1;		// 1
		auto handler = locker.lock(0, 1);
		sleep(2);
		step = 3;		// 3
		locker.unlock(handler);
	});

	sleep(1);
	EXPECT_EQ(1, step);
	step= 2;		// 2
	auto h2 = locker.lock(1, 3);
	EXPECT_EQ(3, step);
	locker.unlock(h2);

	f.get();
}

TEST(RangeLock, SimpleLockPass)
{
	RangeLock<int> locker;
	int step = 0;
	
	auto f = async([&locker, &step]() {
		step = 1;		// 1
		auto handler = locker.lock(1, 10);
		sleep(2);
		step = 3;		// 3
		locker.unlock(handler);
	});

	sleep(1);
	EXPECT_EQ(1, step);
	step= 2;		// 2
	auto h2 = locker.lock(20, 30);
	EXPECT_EQ(2, step);
	locker.unlock(h2);

	f.get();
}

TEST(RangeLock, BlockedByPreviousWaiting)
{
	RangeLock<int> locker;
	int step = 0;

	auto f1 = async([&locker, &step]() {
		step = 1;		// 1
		auto handler = locker.lock(0, 10);
		sleep(3);
		step = 4;		// 4
		locker.unlock(handler);
	});

	auto f2 = async([&locker, &step]() {
		sleep(1);
		step = 2;		// 2
		auto handler = locker.lock(10, 20);
		EXPECT_EQ(4, step);

		sleep(3);
		step = 5;		// 5
		locker.unlock(handler);
	});

	sleep(2);
	EXPECT_EQ(2, step);
	step= 3;		// 3
	auto h2 = locker.lock(20, 30);
	EXPECT_EQ(5, step);

	locker.unlock(h2);
}

TEST(RangeLock, ReadRead)
{
	RangeLock<int> locker;
	int step = 0;

	auto f = async([&locker, &step]() {
		step = 1;
		auto h = locker.lock(0, 10, false);
		sleep(2);
		step = 2;
		locker.unlock(h);
	});

	sleep(1);
	auto h2 = locker.lock(1, 10, false);
	// If above line is blocked by previous lock, step will be 1. Or it is 2
	EXPECT_EQ(1, step);
	locker.unlock(h2);

	f.get();
}

TEST(RangeLock, ReadWrite)
{
	RangeLock<int> locker;
	int step = 0;

	auto f = async([&locker, &step]() {
		step = 1;
		auto h = locker.lock(0, 10, false);
		sleep(2);
		step = 2;
		locker.unlock(h);
	});

	sleep(1);
	auto h2 = locker.lock(1, 10);
	// If above line is blocked by previous lock, step will be 1. Or it is 2
	EXPECT_EQ(2, step);
	locker.unlock(h2);

	f.get();
}

TEST(RangeLock, WriteRead)
{
	RangeLock<int> locker;
	int step = 0;

	auto f = async([&locker, &step]() {
		step = 1;
		auto h = locker.lock(0, 10);
		sleep(2);
		step = 2;
		locker.unlock(h);
	});

	sleep(1);
	auto h2 = locker.lock(1, 10, false);
	// If above line is blocked by previous lock, step will be 1. Or it is 2
	EXPECT_EQ(2, step);
	locker.unlock(h2);

	f.get();
}

TEST(RangeLock, WriteWrite)
{
	RangeLock<int> locker;
	int step = 0;

	auto f = async([&locker, &step]() {
		step = 1;
		auto h = locker.lock(0, 10);
		sleep(2);
		step = 2;
		locker.unlock(h);
	});

	sleep(1);
	auto h2 = locker.lock(1, 10);
	// If above line is blocked by previous lock, step will be 1. Or it is 2
	EXPECT_EQ(2, step);
	locker.unlock(h2);

	f.get();
}

TEST(RangeLock, ReadWriteRead)
{
	RangeLock<int> locker;
	int step = 0;

	auto f1 = async([&locker, &step]() {
		step = 1;
		auto handler = locker.lock(0, 10, false);
		sleep(3);
		EXPECT_EQ(3, step);
		step = 4;
		locker.unlock(handler);
	});

	auto f2 = async([&locker, &step]() {
		sleep(1);
		step = 2;
		auto handler = locker.lock(0, 10);
		// above line should be blocked until f1 release its lock
		EXPECT_EQ(4, step);
		step = 5;
		sleep(3);
		step = 6;
		locker.unlock(handler);
	});

	sleep(2);
	EXPECT_EQ(2, step);
	step= 3;
	auto h2 = locker.lock(5, 15, false);
	// Write locker should not be starved, so we arrive here only after f2 unlock
	EXPECT_EQ(6, step);
	locker.unlock(h2);

	f1.get();
	f2.get();
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

