#ifndef MPIBROT_TEST_MULTINODE_TRANSMISSABLE_INT_INCLUDED
#define MPIBROT_TEST_MULTINODE_TRANSMISSABLE_INT_INCLUDED


// Internal
#include "mpi/Transmissable.hpp"
#include "mpi/error.hpp"

// External
#include "mpi.h"

// Standard
#include <iostream>


class TransmissableInt : public mpi::Transmissable
{
private:
  int m_value;

public:
  TransmissableInt(int const t_intial_value = -1) :
    m_value(t_intial_value)
  {}

  friend bool operator==(TransmissableInt const & lhs, TransmissableInt const & rhs)
  {
    return (lhs.m_value == rhs.m_value);
  }

  friend bool operator<(TransmissableInt const & lhs, TransmissableInt const & rhs)
  {
    return (lhs.m_value < rhs.m_value);
  }

  operator int()
  {
    return m_value;
  }

  void mpiSend(int const t_destination, int const t_tag, MPI_Comm const t_comm) const
  {
    unsigned char const * const buf = (unsigned char *)&m_value;
    mpi::error::check(MPI_Send(&buf[0], 1, MPI_BYTE, t_destination, t_tag, t_comm));
    mpi::error::check(MPI_Send(&buf[1], 1, MPI_BYTE, t_destination, t_tag, t_comm));
    mpi::error::check(MPI_Send(&buf[2], 1, MPI_BYTE, t_destination, t_tag, t_comm));
    mpi::error::check(MPI_Send(&buf[3], 1, MPI_BYTE, t_destination, t_tag, t_comm));
  }

  void mpiReceive(int const t_source, int const t_tag, MPI_Comm const t_comm)
  {
    MPI_Status status;
    unsigned char * const buf = (unsigned char *)&m_value;
    mpi::error::check(MPI_Recv(&buf[0], 1, MPI_BYTE, t_source, t_tag, t_comm, &status));
    mpi::error::check(MPI_Recv(&buf[1], 1, MPI_BYTE, t_source, t_tag, t_comm, &status));
    mpi::error::check(MPI_Recv(&buf[2], 1, MPI_BYTE, t_source, t_tag, t_comm, &status));
    mpi::error::check(MPI_Recv(&buf[3], 1, MPI_BYTE, t_source, t_tag, t_comm, &status));
  }
};


#endif // MPIBROT_TEST_MULTINODE_TRANSMISSABLE_INT_INCLUDED
