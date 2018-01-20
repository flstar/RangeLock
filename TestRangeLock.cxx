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
		cout<<"1:lock [0, 1]"<<endl;
		auto handler = locker.lock(0, 1);
		cout<<"1:get lock [0, 1]"<<endl;
		sleep(2);
		step = 3;		// 3
		cout<<"1:unlock [0, 1]"<<endl;
		locker.unlock(handler);
		cout<<"1:[0, 1] unlocked"<<endl;
	});

	sleep(1);
	EXPECT_EQ(1, step);
	step= 2;		// 2
	cout<<"2:lock [1, 3]"<<endl;
	auto h2 = locker.lock(1, 3);
	cout<<"2:get lock [1, 3]"<<endl;
	EXPECT_EQ(3, step);
	cout<<"2:unlock [1, 3]"<<endl;
	locker.unlock(h2);
	cout<<"2:[1, 3] unlocked"<<endl;

	f.get();
}

TEST(RangeLock, SimpleLockPass)
{
	RangeLock<int> locker;
	int step = 0;
	
	auto f = async([&locker, &step]() {
		step = 1;		// 1
		cout<<"1:lock [1, 10]"<<endl;
		auto handler = locker.lock(1, 10);
		cout<<"1:get lock [1, 10]"<<endl;
		sleep(2);
		step = 3;		// 3
		cout<<"1:unlock [1, 10]"<<endl;
		locker.unlock(handler);
		cout<<"1:[1, 10] unlocked"<<endl;
	});

	sleep(1);
	EXPECT_EQ(1, step);
	step= 2;		// 2
	cout<<"2:lock [20, 30]"<<endl;
	auto h2 = locker.lock(20, 30);
	cout<<"2:get lock [20, 30]"<<endl;
	EXPECT_EQ(2, step);
	cout<<"2:unlock [20, 30]"<<endl;
	locker.unlock(h2);
	cout<<"2:[20, 30] unlocked"<<endl;

	f.get();
}

TEST(RangeLock, SimpleLockBlockedByPrevious)
{
	RangeLock<int> locker;
	int step = 0;

	auto f1 = async([&locker, &step]() {
		step = 1;		// 1
		cout<<"1:lock [0, 10]"<<endl;
		auto handler = locker.lock(0, 10);
		cout<<"1:get lock [0, 10]"<<endl;
		sleep(3);
		step = 4;		// 4
		cout<<"1:unlock [1, 10]"<<endl;
		locker.unlock(handler);
		cout<<"1:[1, 10] unlocked"<<endl;
	});

	auto f2 = async([&locker, &step]() {
		sleep(1);
		step = 2;		// 2
		cout<<"2:lock [10, 20]"<<endl;
		auto handler = locker.lock(10, 20);
		cout<<"2:get lock [10, 20]"<<endl;
		EXPECT_EQ(4, step);

		sleep(3);
		step = 5;		// 5
		cout<<"2:unlock [10, 200]"<<endl;
		locker.unlock(handler);
		cout<<"2:[10, 20] unlocked"<<endl;
	});

	sleep(2);
	EXPECT_EQ(2, step);
	step= 3;		// 3
	cout<<"3:lock [30, 40]"<<endl;
	auto h2 = locker.lock(20, 30);
	cout<<"3:get lock [30, 40]"<<endl;
	EXPECT_EQ(5, step);

	cout<<"3:unlock [30, 40]"<<endl;
	locker.unlock(h2);
	cout<<"3:[30, 40] unlocked"<<endl;
}


int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

