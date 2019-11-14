#ifndef UTIL_MPI_COMMUNICATOR_INCLUDED
#define UTIL_MPI_COMMUNICATOR_INCLUDED

// Standard
#include <memory>
#include <deque>

// External
#include "mpi.h"

// Internal
#include "util/RefCount.hpp"


// Tag pool defaults
#define UTIL_MPI_COMMUNICATOR_TAG_POOL_SIZE_DEFAULT 256


namespace util
{
  namespace mpi
  {

    class Communicator : public RefCount
    {
    private:
      MPI_Comm m_communicator;


      // Construct a communicator directly from MPI_Comm, only for internal use
      Communicator(MPI_Comm const t_communicator) :
        m_communicator(t_communicator)
      {}


    public:
      Communicator() :
        m_communicator(MPI_COMM_NULL)
      {}


      operator MPI_Comm() const
      {
        return m_communicator;
      }


      static Communicator world()
      {
        MPI_Comm new_communicator;

        MPI_Comm_dup(
          MPI_COMM_WORLD,
          &new_communicator);

        return Communicator(new_communicator);
      }


      static Communicator self()
      {
        MPI_Comm new_communicator;

        MPI_Comm_dup(
          MPI_COMM_SELF,
          &new_communicator);

        return Communicator(new_communicator);
      }


      static Communicator duplicate(Communicator const & t_original)
      {
        MPI_Comm new_communicator;

        MPI_Comm_dup(
          t_original,
          &new_communicator);

        return Communicator(new_communicator);
      }


      static Communicator split(Communicator const & t_original, int const t_colour, int const t_key)
      {
        MPI_Comm new_communicator;

        MPI_Comm_split(
          t_original,
	        t_colour,
	        t_key,
          &new_communicator);

        return Communicator(new_communicator);
      }


      int size() const
      {
        int communicator_size;
        MPI_Comm_size(m_communicator, &communicator_size);
        return communicator_size;
      }


      int rank() const
      {
        int communicator_rank;
        MPI_Comm_rank(m_communicator, &communicator_rank);
        return communicator_rank;
      }


      ~Communicator()
      {
        if(this->lastReference())
        {
          MPI_Comm_free(&m_communicator);
        }
      }
    };

  } // namespace mpi

} // namespace util


#endif // UTIL_MPI_COMMUNICATOR_INCLUDED
