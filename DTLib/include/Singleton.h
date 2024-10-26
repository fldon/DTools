#include <atomic>
#include <memory>


#ifndef SINGLETON_H
#define SINGLETON_H

namespace dtools
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
private:

    static inline  std::unique_ptr<T> singleton_object{};
    static inline  std::atomic_bool was_initialized{false};
};

template<typename T>
T* Singleton<T>::get_object()
{
    if(was_initialized)
    {
        return singleton_object.get();
    }
    was_initialized = true;
    singleton_object = std::make_unique<T>();
    return singleton_object.get();
}

}

#endif
