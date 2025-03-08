#include "concurrency/PriorityMutex.h"

using namespace NS_dtools;
using namespace NS_concurrency::NS_priority_mutex;

void Priority_Mutex::lock(int prioritylvl)
{
    std::unique_lock<decltype(mMut)> lk(mMut);

    lockPriorities.push(prioritylvl);

    while(lockPriorities.top() != prioritylvl)
    {
        mCondV.wait(lk, [&]{
            return !locked && (lockPriorities.top() == prioritylvl);
        });
    }
    locked = true;
}

void Priority_Mutex::unlock()
{
    std::lock_guard<decltype(mMut)> lk(mMut);
    assert(!lockPriorities.empty() && "PriorityMutex::unlock: priority queue is empty!");
    lockPriorities.pop();
    locked = false;
    mCondV.notify_all();
}

