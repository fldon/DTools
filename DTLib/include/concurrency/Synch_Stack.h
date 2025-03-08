#ifndef SYNCH_STACK_H
#define SYNCH_STACK_H

#include <boost/lockfree/stack.hpp>

namespace NS_dtools
{

namespace NS_concurrency
{

/*!
 * \brief Simple synched stack.
 * Uses a lockfree_stack internally and offers a simplified interface
 */
template<typename T>
class Synch_Stack
{
public:
    [[nodiscard]] bool empty() const;
    bool push(const T &IN_element); //blocking, but only in respect to other pushes
    [[nodiscard]] bool pop(T& OUT_element); //non-blocking. retval false if stack was empty
    bool clear(); //non-blocking
private:
    boost::lockfree::stack<T> mInternal_stack;
};


template<typename T>
bool Synch_Stack<T>::push(const T &element)
{
    return mInternal_stack.push(element);
}

template<typename T>
bool Synch_Stack<T>::pop(T& OUT_element)
{
    return mInternal_stack.pop(OUT_element);

}

template<typename T>
bool Synch_Stack<T>::empty() const
{
    return mInternal_stack.empty();
}

template<typename T>
bool Synch_Stack<T>::clear()
{
    return mInternal_stack.consume_all([](const T&){});
}

} //NS_concurrency
} //NS_dtools


#endif // SYNCH_STACK_H
