#ifndef SYNCH_QUEUE_H
#define SYNCH_QUEUE_H

#include <boost/lockfree/queue.hpp>

namespace NS_dtools
{

namespace NS_concurrency
{

/*!
 * \brief Simple synched queue.
 * Uses a lockfree_queue internally and offers a simplified interface
 */
template<typename T>
class Synch_Queue
{
public:
    [[nodiscard]] bool empty() const;
    bool push(const T &IN_element); //blocking, but only in respect to other pushes
    [[nodiscard]] bool pop(T& OUT_element); //non-blocking. retval false if queue was empty
    bool clear(); //non-blocking
private:
    boost::lockfree::queue<T> mInternal_queue;
};


template<typename T>
bool Synch_Queue<T>::push(const T &element)
{
    return mInternal_queue.push(element);
}

template<typename T>
bool Synch_Queue<T>::pop(T& OUT_element)
{
    return mInternal_queue.pop(OUT_element);

}

template<typename T>
bool Synch_Queue<T>::empty() const
{
    return mInternal_queue.empty();
}

template<typename T>
bool Synch_Queue<T>::clear()
{
    return mInternal_queue.consume_all([](const T&){});
}

} //NS_concurrency
} //NS_dtools

#endif // SYNCH_QUEUE_H
