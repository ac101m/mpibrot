// This is a catch module
#include "catch.hpp"


// Internal
#include "util/Distributor.hpp"
#include "mpi/Transmissable.hpp"

// Standard
#include <vector>
#include <memory>
#include <algorithm>


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

  friend bool operator!=(TransmissableInt const & lhs, TransmissableInt const & rhs)
  {
    return !(lhs == rhs);
  }

  friend bool operator>(TransmissableInt const & lhs, TransmissableInt const & rhs)
  {
    return (lhs.m_value > rhs.m_value);
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


SCENARIO(
  "[Distributor] - Single rank test")
{
  unsigned input_queue_length = 4;
  unsigned output_queue_length = 4;

  std::shared_ptr<util::Queue<TransmissableInt>> input_queue(new util::Queue<TransmissableInt>(input_queue_length));
  std::shared_ptr<util::Queue<TransmissableInt>> output_queue(new util::Queue<TransmissableInt>(output_queue_length));

  util::Distributor<TransmissableInt> distributor(input_queue, output_queue, MPI_COMM_SELF);

  GIVEN("A vector of transmissable items")
  {
    unsigned test_vector_length = 256;

    std::vector<TransmissableInt> input_vector = std::vector<TransmissableInt>(test_vector_length);
    std::vector<TransmissableInt> output_vector = std::vector<TransmissableInt>(test_vector_length);

    for(unsigned i = 0; i < test_vector_length; i++)
    {
      input_vector[i] = rand();
    }

    WHEN("The vector is enqueued and dequeued on the same rank")
    {
      std::thread enqueue_thread(&util::Queue<TransmissableInt>::enqueueVector, &(*input_queue), std::ref(input_vector));
      std::thread dequeue_thread(&util::Queue<TransmissableInt>::dequeueVector, &(*output_queue), std::ref(output_vector));

      enqueue_thread.join();
      dequeue_thread.join();

      THEN("The values are preserved")
      {
        std::sort(input_vector.begin(), input_vector.end());
        std::sort(output_vector.begin(), output_vector.end());

        bool vectors_match = (input_vector == output_vector);
        REQUIRE(vectors_match == true);
      }
    }
  }
}

/*
SCENARIO(
  "[Distributor] - Parallel enqueue/dequeue test")
{
  unsigned input_queue_length = 4;
  unsigned output_queue_length = 4;

  std::shared_ptr<util::Queue<TransmissableInt>> input_queue(new util::Queue<TransmissableInt>(input_queue_length));
  std::shared_ptr<util::Queue<TransmissableInt>> output_queue(new util::Queue<TransmissableInt>(output_queue_length));

  mpi::Communicator communicator = mpi::Communicator::world();

  util::Distributor<TransmissableInt> distributor(input_queue, output_queue, 1, 1, 1, communicator);

  GIVEN("A vector of test items")
  {
    unsigned test_vector_length = 65536;

    WHEN("The data is enqueued on one node")
    {


      THEN("Input data matches output data")
      {

      }
    }
  }
}
*/
