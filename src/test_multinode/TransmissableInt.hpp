#ifndef MPIBROT_TEST_MULTINODE_TRANSMISSABLE_INT_INCLUDED
#define MPIBROT_TEST_MULTINODE_TRANSMISSABLE_INT_INCLUDED


// Internal
#include "mpi/Transmissable.hpp"

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
    int err = MPI_Ssend(&m_value, 1, MPI_INT, t_destination, t_tag, t_comm);
    if(err)
    {
      std::cout << "MPI Error, code: " << err << "\n";
      exit(1);
    }
  }

  void mpiReceive(int const t_source, int const t_tag, MPI_Comm const t_comm)
  {
    MPI_Status status;
    int err = MPI_Recv(&m_value, 1, MPI_INT, t_source, t_tag, t_comm, &status);
    if(err)
    {
      std::cout << "MPI Error, code: " << err << "\n";
      exit(1);
    }
  }
};


#endif // MPIBROT_TEST_MULTINODE_TRANSMISSABLE_INT_INCLUDED
