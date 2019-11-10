
// Standard
#include <list>

// External

// Internal
#include "MPIUtil.hpp"


// What range of tags to initialise free tag list with
#define MPIUTIL_TAG_RANGE_START 1024
#define MPIUTIL_TAG_RANGE_END 2047


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


    std::mutex Tag::s_free_tag_mutex;
    std::list<int> Tag::s_free_tags = Tag::initFreeTags(MPIUTIL_TAG_RANGE_START, MPIUTIL_TAG_RANGE_END);

  } // namespace mpi

} // namespace util
