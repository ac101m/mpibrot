#ifndef MPIBROT_SYNCHRONIZED_QUEUE_INCLUDED
#define MPIBROT_SYNCHRONIZED_QUEUE_INCLUDED


// Internal
#include "util/CountingSemaphore.hpp"

// Standard
#include <vector>
#include <memory>
#include <mutex>
#include <utility>


namespace util
{

  // Base class for things that you can Enqueue stuff to
  template<class T>
  class Enqueue
  {
  public:
    virtual void enqueue(T t_data) = 0;

    void enqueueVector(std::vector<T> const & t_data)
    {
      for(unsigned i = 0; i < t_data.size(); i++)
      {
        this->enqueue(t_data[i]);
      }
    }
  };


  // Base class for things that you can dequeue stuff from
  template<class T>
  class Dequeue
  {
  public:
    virtual T dequeue() = 0;
    
    void dequeueVector(std::vector<T> & t_data)
    {
      for(unsigned i = 0; i < t_data.size(); i++)
      {
        t_data[i] = this->dequeue();
      }
    }
  };


  template<class T>
  class Queue : public Enqueue<T>, public Dequeue<T>
  {
  private:
    unsigned m_front = 0;
    unsigned m_back = 0;
    std::vector<T> m_buffer;
    std::unique_ptr<util::CountingSemaphore> m_sem_data;
    std::unique_ptr<util::CountingSemaphore> m_sem_free_space;
    std::mutex m_mutex;

  public:
    Queue(unsigned const t_size) :
      m_front(0),
      m_back(0),
      m_buffer(std::vector<T>(t_size)),
      m_sem_data(new util::CountingSemaphore(0)),
      m_sem_free_space(new util::CountingSemaphore(t_size))
    {}

    void enqueue(T t_data)
    {
      m_sem_free_space->take();

      {
        std::lock_guard<decltype(m_mutex)> lock(m_mutex);
        m_buffer[m_back] = t_data;
        m_back = (m_back + 1) % m_buffer.size();
      }

      m_sem_data->give();
    }

    T dequeue()
    {
      m_sem_data->take();

      T data;
      {
        std::lock_guard<decltype(m_mutex)> lock(m_mutex);
        data = m_buffer[m_front];
        m_buffer[m_front] = T();
        m_front = (m_front + 1) % m_buffer.size();
      }

      m_sem_free_space->give();
      return data;
    }

    unsigned size() const
    {
      return m_buffer.size();
    }
  };

} // namespace util

#endif // MPIBROT_SYNCHRONIZED_QUEUE_INCLUDED
