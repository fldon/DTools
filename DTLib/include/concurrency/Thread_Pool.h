#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <boost/asio.hpp>
#include <DTools/MiscTools.h>

namespace NS_dtools
{

namespace NS_concurrency
{

/*
 * Threadpool should have the following features:
 * -Should be able to return future or be used as "fire and forget"
 * -Should be stoppable & joinable as a whole
 * -Should have a function like "wait-for-all-tasks-to-be-done"
 * -Should be able to attach a thread from within the thread (and also detach it from within the thread?)
 *
 *-Should use the std::jthread class and also join on destruction, if not using boost thread_pool
 *
 *TODO: I suppose in the long run, I should re-implement this from scratch. The boost thread pool is too limiting in its API
 *Perhaps there IS no way to implement a wait_for_empty_task_queue, that does not destroy the performance?
 * */

class Thread_Pool final
{
public:
    Thread_Pool(uint num_threads);
    ~Thread_Pool();

    Thread_Pool(const Thread_Pool &rhs) = delete;
    Thread_Pool(Thread_Pool &&rhs) = delete;
    Thread_Pool& operator=(const Thread_Pool &rhs) = delete;
    Thread_Pool& operator=(Thread_Pool &&rhs) = delete;

/*!
 * \brief Post a function to the pool without getting a return future
 * \param f: Functor
 * \param args: Functor arguments
 */
template<typename Functor, typename... argtypes>
    void post_free(Functor&& f, argtypes&&... args)
    {
    std::lock_guard<std::mutex> guard(m_curr_task_update_mut);
    if(not (m_current_tasks < std::numeric_limits<unsigned long long>::max() - 1) )
    {
        throw NS_dtools::NS_misc::OmegaException<unsigned long long>("Task limit reached: ", m_current_tasks);
    }
    ++m_current_tasks;

    auto bound_functor = std::bind(std::forward<Functor>(f), std::forward<argtypes>(args)...);
    auto task_wrapper = [this, bound_functor=std::move(bound_functor)]() -> void
    {
        bound_functor();
        std::lock_guard<std::mutex> guard(m_curr_task_update_mut);

        if(m_current_tasks == 0)
        {
            throw NS_dtools::NS_misc::BaseOmegaException("Internal Error: m_current_tasks is 0 at decrement!");
        }

        --m_current_tasks;
        m_tasks_done_cv.notify_all();
    };


        boost::asio::post(m_pool, task_wrapper );
    }

/*!
 * \brief Post a functor to the pool.
 * \param f: Functor
 * \param args: Functor arguments
 * \return Returns future with return value.
 */
template<typename Functor, typename... argtypes>
[[nodiscard]] std::future< std::invoke_result_t<Functor, argtypes...> > post(Functor&& f, argtypes&&... args)
    {
        std::lock_guard<std::mutex> guard(m_curr_task_update_mut);
        if(not (m_current_tasks < std::numeric_limits<unsigned long long>::max() - 1) )
        {
            throw NS_dtools::NS_misc::OmegaException<unsigned long long>("Task limit reached: ", m_current_tasks);
        }
        ++m_current_tasks;


        auto bound_functor = std::bind(std::forward<Functor>(f), std::forward<argtypes>(args)...);
        auto task_wrapper = [this, bound_functor=std::move(bound_functor)]() -> std::invoke_result_t<Functor, argtypes...>
        {
            const std::invoke_result_t<Functor, argtypes...> result = bound_functor();
            std::lock_guard<std::mutex> guard(m_curr_task_update_mut);

            if(m_current_tasks == 0)
            {
                //This will crash the executable, but at this point we are fucked anyway
                throw NS_dtools::NS_misc::BaseOmegaException("Internal Error: m_current_tasks is 0 at decrement!");
            }

            --m_current_tasks;
            m_tasks_done_cv.notify_all();

            return result;
        };


        return boost::asio::post(m_pool,
                                 std::packaged_task<std::invoke_result_t<Functor, argtypes...>()>(task_wrapper) );
    }

/*!
 * \brief Stop execution of thread pool.
 * Does not join immediately.
 */
void stop();

/*!
 * \brief Blocks until all pool threads have joined
 */
void join();

/*!
 * \brief Waits until there are no more tasks on the queue.
 * Does not stop execution and does not join.
 */
void wait_for_tasks_done();

/*!
 * \brief Adds the currently executing thread to the thread pool
 */
void attach_current_thread();

/*!
 * \brief Attaches the specified number of threads to the pool
 * \param num_threads
 */
void attach_threads(uint num_threads);


/*!
 * \brief Detaches the specified number of threads to the pool
 * Currently not supported!
 * \param num_threads
 */
void detach_threads(uint num_threads);

private:
    boost::asio::thread_pool m_pool;
    unsigned long long m_thread_count;


    std::mutex m_curr_task_update_mut;
    std::condition_variable m_tasks_done_cv;
    unsigned long long m_current_tasks{0};
};

} //NS_concurrency
} //NS_dtools
#endif // THREAD_POOL_H
