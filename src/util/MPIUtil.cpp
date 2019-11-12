
// Standard
#include <list>

// External

// Internal
#include "MPIUtil.hpp"


namespace util
{
  namespace mpi
  {

    int myRank(MPI_Comm const t_communicator)
    {
      int my_rank;
      MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
      return my_rank;
    }


    int rankCount(MPI_Comm const t_communicator)
    {
      int rank_count;
      MPI_Comm_size(MPI_COMM_WORLD, &rank_count);
      return rank_count;
    }

  } // namespace mpi

} // namespace util
