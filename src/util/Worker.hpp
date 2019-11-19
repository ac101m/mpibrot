#ifndef MPIBROT_WORK_QUEUE_INCLUDED
#define MPIBROT_WORK_QUEUE_INCLUDED


// Internal
#include "util/Queue.hpp"

// Standard
#include <vector>
#include <thread>
#include <memory>


namespace util
{

  template<class T_in>
  class Worker
  {
  private:
    std::shared_ptr<util::Queue<T_in>> m_input_queue;

    std::vector<std::thread> m_worker_threads;

    T_in const m_shutdown_signal_value;


  // Methods
  protected:
    virtual void processWorkItem(T_in t_work_item) = 0;


    void workerMain()
    {
      T_in work_item;

      while((work_item = m_input_queue->dequeue()) != m_shutdown_signal_value)
      {
        processWorkItem(work_item);
      }
    }


  // Methods
  public:
    Worker(
      std::shared_ptr<Queue<T_in>> t_input_queue,
      unsigned const t_thread_count,
      T_in const t_shutdown_signal_value = T_in()) :    // Value of T_in which should initiate worker thread shutdown
      m_input_queue(t_input_queue),
      m_worker_threads(std::vector<std::thread>(t_thread_count)),
      m_shutdown_signal_value(t_shutdown_signal_value)
    {
      for(unsigned i = 0; i < t_thread_count; i++)
      {
        m_worker_threads[i] = std::thread(&util::Worker<T_in>::workerMain, this);
      }
    }


    unsigned threadCount() const
    {
      return m_worker_threads.size();
    }


    ~Worker()
    {
      for(unsigned i = 0; i < m_worker_threads.size(); i++)
      {
        m_input_queue->enqueue(m_shutdown_signal_value);
      }

      for(unsigned i = 0; i < m_worker_threads.size(); i++)
      {
        m_worker_threads[i].join();
      }
    }
  };

} // namespace util


#endif // MPIBROT_WORK_QUEUE_INCLUDED
