#include <future>
#include <gtest/gtest.h>

#include "RangeLock.h"

using namespace std;


TEST(RangeLock, SimpleLock)
{
	RangeLock<int> locker;
	int step = 0;
	
	auto f = async([&locker, &step]() {
		step++;		// 1
		auto handler = locker.lock(0, 1);
		usleep(20);
		step++;		// 3
		locker.unlock(handler);
	});

	usleep(10);
	EXPECT_EQ(1, step);
	step++;		// 2
	auto h2 = locker.lock(1, 3);
	EXPECT_EQ(3, step);
	locker.unlock(h2);
}

TEST(RangeLock, SimplePass)
{
	RangeLock<int> locker;
	int step = 0;
	
	auto f = async([&locker, &step]() {
		step++;		// 1
		auto handler = locker.lock(0, 1);
		usleep(20);
		EXPECT_EQ(3, step);
		locker.unlock(handler);
	});

	usleep(10);
	EXPECT_EQ(1, step);
	step++;		// 2
	auto h2 = locker.lock(2, 4);
	EXPECT_EQ(2, step);
	step++;		// 3
	locker.unlock(h2);
}


int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

