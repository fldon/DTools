#include "PriorityMutex.h"

using namespace NS_dtools;
using namespace NS_concurrency::NS_priority_mutex;

void Shared_Priority_Mutex::lock(int prioritylvl)
{
    std::unique_lock<decltype(mMut)> lk(mMut);

    //Push to list must be synchronized even if main Mutex is shared
    {
        std::lock_guard<decltype(internal_admin_mutex)> queue_lk(internal_admin_mutex);
        lockPriorities.push(prioritylvl);
    }

    while(lockPriorities.top() != prioritylvl)
    {
        mCondV.wait(lk, [&]{
            std::lock_guard<decltype(internal_admin_mutex)> queue_lk(internal_admin_mutex);
            return lock_counter == 0 && (lockPriorities.top() == prioritylvl);
        });
    }

    std::lock_guard<decltype(internal_admin_mutex)> queue_lk(internal_admin_mutex);
    assert(lock_counter == 0);
    lock_counter++;
    is_shared_lock = false;
}

void Shared_Priority_Mutex::unlock()
{
    std::lock_guard<decltype(mMut)> lk(mMut);
    //Pop from list must be synchronized even if main Mutex lock is shared
    std::lock_guard<decltype(internal_admin_mutex)> queue_lk(internal_admin_mutex);

    assert(!lockPriorities.empty() && "PriorityMutex::unlock: priority queue is empty!");
    lockPriorities.pop();
    lock_counter--;
    assert(lock_counter == 0);
    is_shared_lock = false;
    mCondV.notify_all();
}

void Shared_Priority_Mutex::lock_shared(int prioritylvl)
{
    std::shared_lock<decltype(mMut)> lk(mMut);

    //Push to list must be synchronized even if main Mutex is shared
    {
        std::lock_guard<decltype(internal_admin_mutex)> queue_lk(internal_admin_mutex);
        lockPriorities.push(prioritylvl);
    }

    while(lockPriorities.top() != prioritylvl)
    {
        mCondV.wait(lk, [&]{
            std::lock_guard<decltype(internal_admin_mutex)> queue_lk(internal_admin_mutex);
            return (lock_counter == 0 || is_shared_lock) && (lockPriorities.top() == prioritylvl);
        });
    }

    //Setting of counter and is_shared_lock must be synchronized even if main mutex is shared
    std::lock_guard<decltype(internal_admin_mutex)> queue_lk(internal_admin_mutex);
    lock_counter++;
    is_shared_lock = true;
}

void Shared_Priority_Mutex::unlock_shared()
{
    std::shared_lock<decltype(mMut)> lk(mMut);
    //Pop from list must be synchronized even if main Mutex lock is shared
    std::lock_guard<decltype(internal_admin_mutex)> queue_lk(internal_admin_mutex);
    assert(!lockPriorities.empty() && "PriorityMutex::unlock: priority queue is empty!");
    lockPriorities.pop();
    lock_counter--;
    assert(lock_counter >= 0);
    if(lock_counter == 0)
    {
        is_shared_lock = false;
    }

    mCondV.notify_all();
}


void Biased_Shared_Priority_Mutex::lock(int priorityLvl)
{
    Shared_Priority_Mutex::lock(priorityLvl + mBias);
}

void Biased_Shared_Priority_Mutex::lock_shared(int prioritylvl)
{
    Shared_Priority_Mutex::lock_shared(prioritylvl - mBias);
}
