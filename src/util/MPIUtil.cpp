
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

    std::mutex Tag::s_free_tag_mutex;
    std::list<int> Tag::s_free_tags = Tag::initFreeTags(MPIUTIL_TAG_RANGE_START, MPIUTIL_TAG_RANGE_END);

  } // namespace mpi

} // namespace util
