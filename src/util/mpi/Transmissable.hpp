#ifndef MPIBROT_UTIL_MPI_TRANSMISSABLE_INCLUDED
#define MPIBROT_UTIL_MPI_TRANSMISSABLE_INCLUDED


// External
#include "mpi.h"


namespace util
{
  namespace mpi
  {

    // interface for things that can be sent over MPI
    class Transmissable
    {
    public:
      virtual void mpiSend(int const t_destination, int const t_tag, MPI_Comm const t_comm) const = 0;
      virtual void mpiReceive(int const t_source, int const t_tag, MPI_Comm const t_comm) = 0;
    };

  } // namespace mpi

} // namesapce util


#endif // MPIBROT_UTIL_MPI_TRANSMISSABLE_INCLUDED
