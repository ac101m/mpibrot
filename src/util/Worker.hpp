#ifndef MPIBROT_WORK_QUEUE_INCLUDED
#define MPIBROT_WORK_QUEUE_INCLUDED

// Standard
#include <vector>
#include <thread>
#include <memory>

// External

// Internal
#include "util/Queue.hpp"


namespace util
{

  template<class T_in>
  class Worker : public Enqueue<T_in>
  {
  protected:
    std::shared_ptr<util::Queue<T_in>> m_input_queue;

    unsigned const m_max_work_items;
    std::unique_ptr<util::CountingSemaphore> m_guard_semaphore;

    std::vector<std::thread> m_worker_threads;


    // Value of T_in which workers should interpret as a stop signal
    virtual T_in workerExitSignal()
    {
      return T_in();
    }

    // Process an arbitrary work item
    virtual void processWorkItem(T_in t_work_item) = 0;

    void workerMain()
    {
      T_in work_item;

      while((work_item = m_input_queue->dequeue()) != this->workerExitSignal())
      {
        processWorkItem(work_item);
        m_guard_semaphore->give();
      }
    }

  public:
    Worker(
      unsigned const t_thread_count,
      unsigned const t_input_queue_size) :
      m_input_queue(new util::Queue<T_in>(t_input_queue_size)),
      m_max_work_items(t_thread_count + t_input_queue_size),
      m_guard_semaphore(new util::CountingSemaphore(m_max_work_items)),
      m_worker_threads(std::vector<std::thread>(t_thread_count))
    {
      for(unsigned i = 0; i < t_thread_count; i++)
      {
        m_worker_threads[i] = std::thread(&util::Worker<T_in>::workerMain, this);
      }
    }

    void enqueue(T_in t_data)
    {
      m_guard_semaphore->take();
      m_input_queue->enqueue(t_data);
    }

    unsigned threadCount() const
    {
      return m_worker_threads.size();
    }

    ~Worker()
    {
      for(unsigned i = 0; i < m_worker_threads.size(); i++)
      {
        m_guard_semaphore->take();
      }

      for(unsigned i = 0; i < m_worker_threads.size(); i++)
      {
        m_input_queue->enqueue(this->workerExitSignal());
      }

      for(unsigned i = 0; i < m_worker_threads.size(); i++)
      {
        m_worker_threads[i].join();
      }
    }
  };

} // namespace util


#endif // MPIBROT_WORK_QUEUE_INCLUDED
