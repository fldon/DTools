#ifndef SYNCHRONIZEDVALUE_H
#define SYNCHRONIZEDVALUE_H

#include <queue>
#include <shared_mutex>
#include <mutex>
#include <atomic>
#include <cassert>
#include <future>
#include "PriorityMutex.h"

using namespace std::chrono_literals;

namespace NS_dtools
{

using namespace NS_concurrency::NS_priority_mutex;

enum SYNCHRONIZEDVALUEMODE: int {UPDATEINORDER = 0x1, PRIORITIZESET = 0x10, PRIORITIZEGET = 0x100};

/*
 * /brief Wraps an object of type T that is either movable or copyable. Access to the wrapper is synchronized as follows:
 * get() can be called in parallel without blocking if the mode is UPDATEALWAYS and no calls to set happen at the same time.
 * set() blocks when other set(async) or get(async) calls are ongoing at the same time.
 * If the mode is UPDATEINORDER, the values set by set() are saved in a queue and get() returns them in the order they were set. If the queue is empty, the last set value is returned.
 * set and get have async versions that respect the order of calls and otherwise behave the same as their synchronous versions.
 * */
//TODO maybe use bitset/bit field for bit flags instead of uint
template <typename T>
class Synchronized_Value
{
public:
    Synchronized_Value(T&& initval, int mode = 0x0);
    Synchronized_Value() = default;
    ~Synchronized_Value() = default;

    template <typename U>
    void set(U&& val); //Is blocking
    template <typename U>
    void setAsync(U&& val);

   [[maybe_unused]] T get(); //Is blocking
   [[maybe_unused]] std::future<T> getAsync();

    Synchronized_Value& operator =(const Synchronized_Value&) = delete;
    Synchronized_Value(const Synchronized_Value&) = delete;
    Synchronized_Value& operator =(const Synchronized_Value&&) = delete;
    Synchronized_Value(const Synchronized_Value&&) = delete;
private:
    template <typename U>
    void setInternal(U&& val, unsigned int orderIdx);

    template <typename U>
    void fill_value_queue(U&& val);
    void fill_active_val_from_queue();

    std::queue<T> outstanding_input_vals;
    T active_val;
    int mMode{0x0};

    std::atomic_uint setStartOrderIdx{1};
    std::atomic_uint setEndOrderIdx{0};
    mutable Biased_Shared_Priority_Mutex activeValMut;
    mutable std::mutex inputQueueMut;
    mutable std::mutex setAsyncOrderMut;
    mutable std::condition_variable setAsyncCond;
};

template <typename T>
Synchronized_Value<T>::Synchronized_Value(T&& initval, int mode)
:   active_val(std::forward<T>(initval)),
    mMode(mode),
    //Set the mode to either +1 (priority for setter) or -1 (priority for getter) or 0 if both or no flag is set in mode
    activeValMut( (mode & SYNCHRONIZEDVALUEMODE::PRIORITIZESET / SYNCHRONIZEDVALUEMODE::PRIORITIZESET) - (mode & SYNCHRONIZEDVALUEMODE::PRIORITIZEGET / SYNCHRONIZEDVALUEMODE::PRIORITIZEGET) )
{

}

template <typename T> template <typename U>
void Synchronized_Value<T>::setAsync(U&& val)
{
    static_assert(std::is_same<std::decay_t<U>,std::decay_t<T>>::value,
            "SynchronizedValue::setAsync: U must be the same as T");

    std::async(std::launch::async, set(), std::forward<U> (val), setStartOrderIdx++);
}

template <typename T> template <typename U>
void Synchronized_Value<T>::set(U&& val)
{
    static_assert(std::is_same<std::decay_t<U>,std::decay_t<T>>::value,
            "SynchronizedValue::set: U must be the same as T");

    setInternal(std::forward<U>(val), setStartOrderIdx++);
}

template <typename T> template <typename U>
void Synchronized_Value<T>::setInternal(U&& val, unsigned int orderIdx)
{
    static_assert(std::is_same<std::decay_t<U>,std::decay_t<T>>::value,
            "SynchronizedValue::setInternal: U must be the same as T");
    assert(!(setEndOrderIdx != orderIdx+1 && setEndOrderIdx > orderIdx) && "SynchronizedValue::setInternal: setEndOrderIdx is out of order for attempted set call");

    bool setComplete = false;
    do
    {
        std::unique_lock<std::mutex> lk(setAsyncOrderMut);
        //Will only wait if another operation comes first in the order
        setAsyncCond.wait(lk, [this, orderIdx=orderIdx] () {return orderIdx == setEndOrderIdx + 1;});
        if(orderIdx == setEndOrderIdx + 1)
        {

            if(mMode & UPDATEINORDER)
            {
                fill_value_queue(std::forward<U>(val));
            }
            else
            {
                std::lock_guard<decltype(activeValMut)> lk(activeValMut);
                active_val = std::forward<U>(val);
            }
            setEndOrderIdx++;
            setComplete = true;
            setAsyncCond.notify_all();
        }
    } while(!setComplete);
}

template <typename T>
T Synchronized_Value<T>::get()
{
    if((mMode & UPDATEINORDER) && !outstanding_input_vals.empty())
    {
        fill_active_val_from_queue();
    }
    std::shared_lock<decltype(activeValMut)> readlock(activeValMut);
    return active_val;
}

template <typename T>
std::future<T> Synchronized_Value<T>::getAsync()
{
    return std::async(std::launch::async, get());
}

template <typename T> template <typename U>
void Synchronized_Value<T>::fill_value_queue(U&& val)
{
    static_assert(std::is_same<std::decay_t<U>,std::decay_t<T>>::value,
            "SynchronizedValue::fill_value_queue: U must be the same as T");
    {
        std::lock_guard<std::mutex> lk(inputQueueMut);
        outstanding_input_vals.push(std::forward<U>(val));
    }
}

template <typename T>
void Synchronized_Value<T>::fill_active_val_from_queue()
{
    if(outstanding_input_vals.empty())
    {
        return;
    }
    std::lock_guard<std::mutex> lkq(inputQueueMut);
    T &fillvalue = outstanding_input_vals.front();
    std::lock_guard<decltype(activeValMut)> lk(activeValMut);
    active_val = fillvalue;
    outstanding_input_vals.pop();
}

}
#endif
