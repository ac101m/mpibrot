#ifndef MPIBROT_SYNCHRONIZED_QUEUE_INCLUDED
#define MPIBROT_SYNCHRONIZED_QUEUE_INCLUDED

// Standard
#include <vector>
#include <memory>
#include <mutex>

// Internal
#include "util/CountingSemaphore.hpp"


namespace util
{

  template<class T>
  class Queue
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

    void enqueueVector(std::vector<T> const& data)
    {
      for(unsigned i = 0; i < data.size(); i++)
      {
        this->enqueue(data[i]);
      }
    }

    void dequeueVector(std::vector<T>& data)
    {
      for(unsigned i = 0; i < data.size(); i++)
      {
        data[i] = this->dequeue();
      }
    }

    unsigned size() const
    {
      return m_buffer.size();
    }
  };

} // namespace util

#endif // MPIBROT_SYNCHRONIZED_QUEUE_INCLUDED