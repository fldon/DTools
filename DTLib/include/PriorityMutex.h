#include <shared_mutex>
#include <mutex>
#include <condition_variable>
#include <cassert>
#include <queue>


#ifndef PRIORITYMUTEX_H
#define PRIORITYMUTEX_H

namespace NS_dtools
{

namespace NS_concurrency
{

namespace NS_priority_mutex
{

constexpr int DEFAULT_PRIORITY = 100;

class Priority_Mutex {
public:
    Priority_Mutex() = default;
    ~Priority_Mutex() { assert(!locked); }

    Priority_Mutex(const Priority_Mutex&) = delete;
    Priority_Mutex operator=(const Priority_Mutex&) = delete;
    Priority_Mutex(Priority_Mutex&&) = delete;
    Priority_Mutex operator=(Priority_Mutex &&) = delete;

    void lock(int priorityLvl = DEFAULT_PRIORITY);

    void unlock();

private:
    std::condition_variable mCondV;
    std::mutex mMut;
    bool locked{false};
    std::priority_queue<int> lockPriorities;
};

/*!
 * \brief Shared and non-shared locking with priorities.
 *        Locking with the same priority using lock_shared creates a normal shared lock.
 *        If the priority is not high enough, or the current lock is not shared, the call blocks.
 *        The default value is 100.
 */
class Shared_Priority_Mutex
{
public:
    Shared_Priority_Mutex() = default;
    virtual ~Shared_Priority_Mutex() { assert(lock_counter == 0); }

    Shared_Priority_Mutex(const Shared_Priority_Mutex&) = delete;
    Shared_Priority_Mutex operator=(const Shared_Priority_Mutex&) = delete;
    Shared_Priority_Mutex(Shared_Priority_Mutex&&) = delete;
    Shared_Priority_Mutex operator=(Shared_Priority_Mutex &&) = delete;

    virtual void lock(int priorityLvl = DEFAULT_PRIORITY);
    virtual void lock_shared(int priorityLvl = DEFAULT_PRIORITY);

    virtual void unlock();
    virtual void unlock_shared();


private:
    std::condition_variable_any mCondV;
    std::shared_mutex mMut;
    std::mutex internal_admin_mutex; //used to manipulate internal queue and is_shared_lock even if shared lock is used
    int lock_counter{0}; //How many clients lock the main mutex right now?
    bool is_shared_lock{false};
    std::priority_queue<int> lockPriorities;
};


/*!
 * \brief A biased shared priority mutex. Can be set with a bias towards shared locks (negative values) or unique locks (positive values) given by an int.
 *  This bias is added / subtracted from the priority of lock() and lock_shared() calls.
 *  The default value is 100 with no bias.
 */
class Biased_Shared_Priority_Mutex : public Shared_Priority_Mutex
{
public:
    Biased_Shared_Priority_Mutex(int IN_bias) : mBias(IN_bias) {};
    ~Biased_Shared_Priority_Mutex() override = default;

    Biased_Shared_Priority_Mutex(const Biased_Shared_Priority_Mutex&) = delete;
    Biased_Shared_Priority_Mutex operator=(const Biased_Shared_Priority_Mutex&) = delete;
    Biased_Shared_Priority_Mutex(Biased_Shared_Priority_Mutex&&) = delete;
    Biased_Shared_Priority_Mutex operator=(Biased_Shared_Priority_Mutex &&) = delete;

    void lock(int priorityLvl = DEFAULT_PRIORITY) override;
    void lock_shared(int priorityLvl = DEFAULT_PRIORITY) override;

private:
    int mBias{0};

};



}

}
}
#endif
