#include <atomic>
#include <memory>


#ifndef SINGLETON_H
#define SINGLETON_H

namespace NS_dtools
{

/*!
 * \brief Simple singleton. Requires Objects to be default constructable.
 */
template<typename T>
class Singleton
{
public:
    [[nodiscard]] static T* get_object();

    Singleton() = default;

    Singleton(const Singleton& rhs) = delete;
    Singleton(Singleton&& rhs) = delete;
    Singleton& operator=(const Singleton& rhs) = delete;
    Singleton& operator=(Singleton&& rhs) = delete;
};

template<typename T>
T* Singleton<T>::get_object()
{
    static T singleton_object;
    return &singleton_object;
}

}

#endif
