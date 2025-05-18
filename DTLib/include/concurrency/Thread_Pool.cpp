#include <DTools/concurrency/Thread_Pool.h>

namespace NS_dtools
{

namespace NS_concurrency
{

Thread_Pool::Thread_Pool(uint num_threads)
    :m_pool(num_threads), m_thread_count(num_threads)
{
}

Thread_Pool::~Thread_Pool()
{
    //Stop the threads; joining is implicit (with boost pool stop is also implicit)
    stop();
}

void Thread_Pool::stop()
{
    m_pool.stop();
}

void Thread_Pool::join()
{
    m_pool.join();
}

void Thread_Pool::wait_for_tasks_done()
{
    std::unique_lock<std::mutex> lock(m_curr_task_update_mut);
    m_tasks_done_cv.wait(lock, [this](){return m_current_tasks == 0;});
}

void Thread_Pool::attach_current_thread()
{
    m_pool.attach();
}

void Thread_Pool::attach_threads(uint num_threads)
{
    //Not currently supported
    assert(false);
}

void Thread_Pool::detach_threads(uint num_threads)
{
    //Not currently supported
    assert(false);
}


} //NS_concurrency
} //NS_dtools
