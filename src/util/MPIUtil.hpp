#ifndef MPIBROT_MPI_UTIL_INCLUDED
#define MPIBROT_MPI_UTIL_INCLUDED


// Standard
#include <list>
#include <iostream>
#include <mutex>

// External
#include "mpi.h"

// Internal
#include "util/RefCount.hpp"


namespace util
{
  namespace mpi
  {

    int myRank(MPI_Comm const t_communicator = MPI_COMM_WORLD);

    int rankCount(MPI_Comm const t_communicator = MPI_COMM_WORLD);


    class Tag : private RefCount
    {
    private:
      int const m_tag_value;

      static std::mutex s_free_tag_mutex;
      static std::list<int> s_free_tags;


      static std::list<int> initFreeTags(unsigned const min, unsigned const max)
      {
        std::list<int> free_tag_list;

        for(unsigned i = min; i <= max; i++)
        {
          free_tag_list.push_back(i);
        }

        return free_tag_list;
      }


      static int allocateTag()
      {
        std::lock_guard<decltype(s_free_tag_mutex)> lock(s_free_tag_mutex);

        if(s_free_tags.size() == 0)
        {
          std::cout << "Error, message tags depleted. Nothing to be done.\n";
          exit(1);
        }

        int tag_value = s_free_tags.front();
        s_free_tags.pop_front();

        return tag_value;
      }


      static void freeTag(int const t_tag_value)
      {
        std::lock_guard<decltype(s_free_tag_mutex)> lock(s_free_tag_mutex);

        s_free_tags.push_front(t_tag_value);
      }

    public:
      Tag() :
        m_tag_value(this->allocateTag())
      {}

      operator int() const
      {
        return m_tag_value;
      }

      ~Tag()
      {
        if(this->lastReference())
        {
          this->freeTag(m_tag_value);
        }
      }
    };

  } // namespace mpi

} // namespace util


#endif // MPIBROT_MPI_UTIL_INCLUDED
