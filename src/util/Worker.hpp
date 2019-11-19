#ifndef MPIBROT_WORK_QUEUE_INCLUDED
#define MPIBROT_WORK_QUEUE_INCLUDED


// Internal
#include "util/Queue.hpp"

// Standard
#include <vector>
#include <thread>
#include <memory>


// Worker exit signal value
#define MPIBROT_WORKER_EXIT_SIGNAL -1


namespace util
{

  template<class T_in>
  class Worker
  {
  private:
    std::shared_ptr<util::Queue<T_in>> m_input_queue;

    std::vector<std::thread> m_worker_threads;


  // Methods
  private:
    void workerMain()
    {
      std::pair<int, T_in> data_signal_pair;

      while(1)
      {
        data_signal_pair = m_input_queue->dequeueWithSignal();

        if(data_signal_pair.first == MPIBROT_WORKER_EXIT_SIGNAL)
        {
          break;
        }

        processWorkItem(data_signal_pair.second);
      }
    }


  // Methods
  protected:
    virtual void processWorkItem(T_in t_work_item) = 0;


  // Methods
  public:
    Worker(
      std::shared_ptr<Queue<T_in>> t_input_queue,
      unsigned const t_thread_count) :
      m_input_queue(t_input_queue),
      m_worker_threads(std::vector<std::thread>(t_thread_count))
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
        m_input_queue->enqueueWithSignal(std::make_pair(MPIBROT_WORKER_EXIT_SIGNAL, T_in()));
      }

      for(unsigned i = 0; i < m_worker_threads.size(); i++)
      {
        m_worker_threads[i].join();
      }
    }
  };

} // namespace util


#endif // MPIBROT_WORK_QUEUE_INCLUDED
