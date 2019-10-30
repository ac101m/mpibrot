#ifndef MPIBROT_COUNTING_SEMAPHORE_INCLUDED
#define MPIBROT_COUNTING_SEMAPHORE_INCLUDED

// Standard
#include <mutex>
#include <condition_variable>


namespace util
{

  // Custom counting semaphore implementation
  // Because the standard library doesn't have them for some reason...
  class CountingSemaphore
  {
  private:
    std::mutex m_mutex;
    std::condition_variable m_cv;
    unsigned m_count;

  public:
    CountingSemaphore(unsigned const t_initial_count = 0) :
      m_count(t_initial_count)
    {}

    inline void give()
    {
      std::lock_guard<decltype(m_mutex)> lock(m_mutex);
      m_count++;
      m_cv.notify_one();
    }

    inline void take()
    {
      std::unique_lock<decltype(m_mutex)> lock(m_mutex);
      while(!m_count)
      {
        m_cv.wait(lock);
      }
      m_count--;
    }

    inline bool tryTake()
    {
      std::lock_guard<decltype(m_mutex)> lock(m_mutex);
      if(m_count)
      {
        m_count--;
        return true;
      }
      return false;
    }

    inline decltype(m_count) count() const
    {
      return m_count;
    }
  };

} // namespace util

#endif // MPIBROT_COUNTING_SEMAPHORE_INCLUDED
