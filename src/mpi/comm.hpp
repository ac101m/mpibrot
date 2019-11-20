#ifndef MPIBROT_UTIL_MPI_COMM_INCLUDED
#define MPIBROT_UTIL_MPI_COMM_INCLUDED


// Internal
#include "mpi/error.hpp"

// External
#include "mpi.h"


namespace mpi
{
  namespace comm
  {

    inline int size(MPI_Comm const t_comm)
    {
      int size;
      mpi::error::check(MPI_Comm_size(t_comm, &size));
      return size;
    }


    inline int rank(MPI_Comm const t_comm)
    {
      int rank;
      mpi::error::check(MPI_Comm_rank(t_comm, &rank));
      return rank;
    }


    inline MPI_Comm duplicate(MPI_Comm const t_comm_original)
    {
      MPI_Comm new_comm;
      mpi::error::check(MPI_Comm_dup(t_comm_original, &new_comm));
      return new_comm;
    }


    inline MPI_Comm split(MPI_Comm const t_comm_original, int const t_colour, int const t_key)
    {
      MPI_Comm new_comm;
      mpi::error::check(MPI_Comm_split(t_comm_original, t_colour, t_key, &new_comm));
      return new_comm;
    }

  } // namespace comm

} // namespace mpi


#endif // MPIBROT_UTIL_MPI_COMM_INCLUDED
