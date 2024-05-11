//TODO: Also make versions with priority mutexes; one with higher priority for the set calls and one with higher priority for the get calls

#ifndef SYNCHRONIZEDVALUE_H
#define SYNCHRONIZEDVALUE_H

#include <queue>
#include <array>
#include <shared_mutex>
#include <mutex>
#include <atomic>
#include <cassert>
#include <chrono>
#include <future>
#include <thread>

using namespace std::chrono_literals;

namespace SynchonizedValue
{

enum SYNCHRONIZEDVALUEMODE {UPDATEINORDER = 0x1, PRIORITIZESET = 0x10, PRIORITIZEGET = 0x100};
/*
 * /brief Wraps an object of type T that is either movable or copyable. Access to the wrapper is synchronized as follows:
 * get() can be called in parallel without blocking if the mode is UPDATEALWAYS and no calls to set happen at the same time.
 * set() blocks when other set(async) or get(async) calls are ongoing at the same time.
 * If the mode is UPDATEINORDER, the values set by set() are saved in a queue and get() returns them in the order they were set. If the queue is empty, the last set value is returned.
 * set and get have async versions that respect the order of calls and otherwise behave the same as their synchronous versions.
 * */
template <typename T>
class SynchronizedValue
{
public:
    SynchronizedValue(T&& initval, unsigned int mode = 0x0);

    template <typename U>
    void set(U&& val); //Is blocking
    template <typename U>
    void setAsync(U&& val);

    T get(); //Is blocking
    std::future<T> getAsync();

    SynchronizedValue& operator =(const SynchronizedValue&) = delete;
    SynchronizedValue(const SynchronizedValue&) = delete;
    SynchronizedValue& operator =(const SynchronizedValue&&) = delete;
    SynchronizedValue(const SynchronizedValue&&) = delete;
private:
    template <typename U>
    void setInternal(U&& val, unsigned int orderIdx);

    template <typename U>
    void fill_value_queue(U&& val);
    void fill_active_val_from_queue();

    std::queue<T> outstanding_input_vals;
    T active_val;
    unsigned int mMode;

    std::atomic_uint setStartOrderIdx{1};
    std::atomic_uint setEndOrderIdx{0};
    mutable std::shared_timed_mutex active_val_mut;
    mutable std::mutex inputQueue_mut;
    mutable std::mutex setAsync_mut;
    mutable std::condition_variable setAsync_cond;
};

template <typename T>
SynchronizedValue<T>::SynchronizedValue(T&& initval, unsigned int mode)
:   mMode(mode),
    active_val(std::forward<T>(initval))
{

}

template <typename T> template <typename U>
void SynchronizedValue<T>::setAsync(U&& val)
{
    static_assert(std::is_same<std::decay_t<U>,std::decay_t<T>>::value,
            "SynchronizedValue::setAsync: U must be the same as T");

    std::async(std::launch::async, set(), std::forward<U> (val), setStartOrderIdx++);
}

template <typename T> template <typename U>
void SynchronizedValue<T>::set(U&& val)
{
    static_assert(std::is_same<std::decay_t<U>,std::decay_t<T>>::value,
            "SynchronizedValue::set: U must be the same as T");

    setInternal(std::forward<U>(val), setStartOrderIdx++);
}

template <typename T> template <typename U>
void SynchronizedValue<T>::setInternal(U&& val, unsigned int orderIdx)
{
    static_assert(std::is_same<std::decay_t<U>,std::decay_t<T>>::value,
            "SynchronizedValue::set: U must be the same as T");
    assert(!(setEndOrderIdx != orderIdx+1 && setEndOrderIdx > orderIdx) && "SynchronizedValue::setInternal: setEndOrderIdx is out of order for attempted set call");

    bool setComplete = false;
    do
    {
        std::unique_lock<std::mutex> lk(setAsync_mut);
        setAsync_cond.wait(lk, [this, orderIdx=orderIdx] () {return orderIdx == setEndOrderIdx + 1;});
        if(orderIdx == setEndOrderIdx + 1)
        {

            if(mMode & UPDATEINORDER)
            {
                fill_value_queue(std::forward<U>(val));
            }
            else
            {
                std::lock_guard<std::shared_timed_mutex> lk(active_val_mut);
                active_val = std::forward<U>(val);
            }
            setEndOrderIdx++;
            setComplete = true;
        }
    } while(!setComplete);
}

template <typename T>
T SynchronizedValue<T>::get()
{
    if((mMode & UPDATEINORDER) && !outstanding_input_vals.empty())
    {
        fill_active_val_from_queue();
    }
    std::shared_lock<std::shared_timed_mutex> readlock(active_val_mut);
    return active_val;
}

template <typename T>
std::future<T> SynchronizedValue<T>::getAsync()
{
    return std::async(std::launch::async, get());
}

template <typename T> template <typename U>
void SynchronizedValue<T>::fill_value_queue(U&& val)
{
    static_assert(std::is_same<std::decay_t<U>,std::decay_t<T>>::value,
            "SynchronizedValue::fill_value_queue: U must be the same as T");
    {
        std::lock_guard<std::mutex> lk(inputQueue_mut);
        outstanding_input_vals.push(std::forward<U>(val));
    }
}

template <typename T>
void SynchronizedValue<T>::fill_active_val_from_queue()
{
    if(outstanding_input_vals.empty())
    {
        return;
    }
    std::lock_guard<std::mutex> lkq(inputQueue_mut);
    T &fillvalue = outstanding_input_vals.front();
    std::lock_guard<std::shared_timed_mutex> lk(active_val_mut);
    active_val = fillvalue;
    outstanding_input_vals.pop();
}

}
#endif
