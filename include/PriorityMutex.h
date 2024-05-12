#include <thread>
#include <mutex>
#include <condition_variable>
#include <cassert>
#include <queue>


#ifndef PRIORITYMUTEX_H
#define PRIORITYMUTEX_H

namespace dtools
{

class PriorityMutex {
public:
  PriorityMutex() : locked(false) {}
  ~PriorityMutex() { assert(!locked); }

  PriorityMutex(PriorityMutex&) = delete;
  PriorityMutex operator=(PriorityMutex&) = delete;

  void lock(int priorityLvl = 100);

  void unlock();

private:
    std::condition_variable mCondV;
    std::mutex mMut;
    bool locked;
    std::priority_queue<int> lockPriorities;
};


class SharedPriorityMutex
{
    //TODO: create shared priority mutex. But how would that work? Same as normal priority mutex but with separate functions for lock_shared?
};

}

#endif
