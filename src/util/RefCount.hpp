#ifndef MPIBROT_REF_COUNT_INCLUDED
#define MPIBROT_REF_COUNT_INCLUDED


// Standard
#include <mutex>


namespace util
{

  class RefCount
  {
  private:
    unsigned * m_shared_count_ptr;
    std::mutex m_counter_lock;


    void Increment(void)
    {
      std::lock_guard<decltype(m_counter_lock)> lock(m_counter_lock);
      (*this->m_shared_count_ptr)++;
    }

    void Decrement(void)
    {
      std::lock_guard<decltype(m_counter_lock)> lock(m_counter_lock);
      (*this->m_shared_count_ptr)--;
    }

  public:
    RefCount(void)
    {
      m_shared_count_ptr = new unsigned(0);
    }

    RefCount(RefCount const & other)
    {
      m_shared_count_ptr = other.m_shared_count_ptr;
      this->Increment();
    }

    RefCount& operator=(RefCount const & other)
    {
      if(this == &other)
      {
        return *this;
      }
      else
      {
        this->m_shared_count_ptr = other.m_shared_count_ptr;
      }
    }

    unsigned getReferenceCount(void) const
    {
      return *this->m_shared_count_ptr;
    }

    bool lastReference(void) const
    {
      return (*this->m_shared_count_ptr == 0);
    }

    ~RefCount(void)
    {
      if(this->getReferenceCount() != 0)
      {
        this->Decrement();
      }
      else
      {
        delete this->m_shared_count_ptr;
      }
    }
  };

}

#endif // MPIBROT_REF_COUNT_INCLUDED
