#include "PriorityMutex.h"

using namespace dtools;

void PriorityMutex::lock(int prioritylvl)
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

void PriorityMutex::unlock()
{
    std::lock_guard<decltype(mMut)> lk(mMut);
    assert(!lockPriorities.empty() && "PriorityMutex::unlock: priority queue is empty!");
    lockPriorities.pop();
    locked = false;
    mCondV.notify_all();
}

