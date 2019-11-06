#ifndef MPIBROT_WORK_QUEUE_INCLUDED
#define MPIBROT_WORK_QUEUE_INCLUDED

// Standard
#include <vector>
#include <thread>
#include <memory>

// External

// Internal
#include "util/SyncQueue.hpp"


namespace util
{

  class WorkItem
  {
  public:
    virtual void execute() = 0;
  };


  class WorkQueue
  {
  private:
    std::shared_ptr<util::SyncQueue<std::shared_ptr<WorkItem>>> m_input_queue;
    std::shared_ptr<util::SyncQueue<std::shared_ptr<WorkItem>>> m_output_queue;

    unsigned const m_max_work_items;
    std::unique_ptr<util::CountingSemaphore> m_guard_semaphore;

    std::vector<std::thread> m_worker_threads;


    void workerMain()
    {
      std::shared_ptr<WorkItem> work_item;

      while((work_item = m_input_queue->dequeue()) != nullptr)
      {
        work_item->execute();
        m_output_queue->enqueue(work_item);
      }
    }

  public:
    WorkQueue(
      unsigned const t_thread_count = 1,
      unsigned const t_input_queue_length = 4,
      unsigned const t_output_queue_length = 4) :
      m_input_queue(new util::SyncQueue<std::shared_ptr<WorkItem>>(t_input_queue_length)),
      m_output_queue(new util::SyncQueue<std::shared_ptr<WorkItem>>(t_output_queue_length)),
      m_max_work_items(t_input_queue_length + t_thread_count),
      m_guard_semaphore(new util::CountingSemaphore(m_max_work_items))
    {
      for(unsigned i = 0; i < t_thread_count; i++)
      {
        m_worker_threads.push_back(std::thread(&util::WorkQueue::workerMain, this));
      }
    }

    template<class T>
    inline void enqueue(std::shared_ptr<T> t_work_item)
    {
      m_guard_semaphore->take();
      m_input_queue->enqueue(std::static_pointer_cast<WorkItem>(t_work_item));
    }

    template<class T>
    inline std::shared_ptr<T> dequeue()
    {
      std::shared_ptr<T> work_item = std::static_pointer_cast<T>(m_output_queue->dequeue());
      m_guard_semaphore->give();
      return work_item;
    }

    template<class T>
    void enqueueVector(std::vector<std::shared_ptr<T>>& t_work_items_in)
    {
      for(unsigned i = 0; i < t_work_items_in.size(); i++)
      {
        this->enqueue<T>(t_work_items_in[i]);
      }
    }

    template<class T>
    void dequeueVector(std::vector<std::shared_ptr<T>>& t_work_items_out)
    {
      for(unsigned i = 0; i < t_work_items_out.size(); i++)
      {
        t_work_items_out[i] = this->dequeue<T>();
      }
    }

    unsigned threadCount() const
    {
      return m_worker_threads.size();
    }

    unsigned bufferSize() const
    {
      return m_input_queue->size();
    }

    ~WorkQueue()
    {
      // Wait for all work to be dequeued
      for(unsigned i = 0; i < m_max_work_items; i++)
      {
        m_guard_semaphore->take();
      }

      // Shutdown the worker threads
      for(unsigned i = 0; i < m_worker_threads.size(); i++)
      {
        m_input_queue->enqueue(nullptr);
      }

      for(unsigned i = 0; i < m_worker_threads.size(); i++)
      {
        m_worker_threads[i].join();
      }
    }
  };

} // namespace util


#endif // MPIBROT_WORK_QUEUE_INCLUDED
