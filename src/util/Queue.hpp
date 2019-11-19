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

  // Base class for things that you can enqueue stuff to
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
    std::vector<std::pair<int, T>> m_buffer;
    std::unique_ptr<util::CountingSemaphore> m_sem_data;
    std::unique_ptr<util::CountingSemaphore> m_sem_free_space;
    std::mutex m_mutex;


  public:
    Queue(unsigned const t_size) :
      m_front(0),
      m_back(0),
      m_buffer(std::vector<std::pair<int, T>>(t_size, std::make_pair(0, T()))),
      m_sem_data(new util::CountingSemaphore(0)),
      m_sem_free_space(new util::CountingSemaphore(t_size))
    {}


    inline void enqueueWithSignal(std::pair<int, T> const t_signal_data_pair)
    {
      m_sem_free_space->take();

      {
        std::lock_guard<decltype(m_mutex)> lock(m_mutex);
        m_buffer[m_back] = t_signal_data_pair;
        m_back = (m_back + 1) % m_buffer.size();
      }

      m_sem_data->give();
    }


    inline void enqueue(T const t_data)
    {
      std::pair<int, T> signal_data_pair = std::make_pair(0, t_data);
      this->enqueueWithSignal(signal_data_pair);
    }


    inline std::pair<int, T> dequeueWithSignal()
    {
      m_sem_data->take();

      std::pair<int, T> signal_data_pair;
      {
        std::lock_guard<decltype(m_mutex)> lock(m_mutex);
        signal_data_pair = m_buffer[m_front];
        m_buffer[m_front] = std::make_pair(0, T());
        m_front = (m_front + 1) % m_buffer.size();
      }

      m_sem_free_space->give();

      return signal_data_pair;
    }


    inline T dequeue()
    {
      std::pair<int, T> signal_data_pair = this->dequeueWithSignal();
      return signal_data_pair.second;
    }


    unsigned size() const
    {
      return m_buffer.size();
    }
  };

} // namespace util

#endif // MPIBROT_SYNCHRONIZED_QUEUE_INCLUDED
