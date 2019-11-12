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

  } // namespace mpi

} // namespace util


#endif // MPIBROT_MPI_UTIL_INCLUDED
