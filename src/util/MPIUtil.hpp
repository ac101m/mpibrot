#ifndef MPIBROT_MPI_UTIL_INCLUDED
#define MPIBROT_MPI_UTIL_INCLUDED


// External
#include "mpi.h"


namespace util::mpi
{

  int myRank(MPI_Comm t_communicator = MPI_COMM_WORLD)
  {
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    return my_rank;
  }


  int rankCount(MPI_Comm t_communicator = MPI_COMM_WORLD)
  {
    int rank_count;
    MPI_Comm_size(MPI_COMM_WORLD, &rank_count);
    return rank_count;
  }

} // namespace util


#endif // MPIBROT_MPI_UTIL_INCLUDED
