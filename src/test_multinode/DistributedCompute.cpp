// This is a catch module
#include "catch.hpp"


// Internal
#include "util/Scatterer.hpp"
#include "util/Gatherer.hpp"
#include "util/Worker.hpp"
#include "mpi/Transmissable.hpp"

// External
#include "mpi.h"

// Standard
#include <vector>


// Ackermanns function (yuck)
unsigned ack(unsigned m, unsigned n)
{
  if (m == 0) return n + 1;
  if (n == 0) return ack(m - 1, 1);
  return ack(m - 1, ack(m, n - 1));
}


class AckermannInput : public mpi::Transmissable
{
public:
  unsigned m = 0;
  unsigned n = 0;

  AckermannInput() : m(0), n(0)
  {}

  AckermannInput(unsigned const t_m, unsigned const t_n) : m(t_m), n(t_n)
  {}

  void mpiSend(int const t_destination, int const t_tag, MPI_Comm const t_comm) const
  {
    mpi::error::check(MPI_Send(&m, 1, MPI_INT, t_destination, t_tag, t_comm));
    mpi::error::check(MPI_Send(&n, 1, MPI_INT, t_destination, t_tag, t_comm));
  }

  void mpiReceive(int const t_source, int const t_tag, MPI_Comm const t_comm)
  {
    mpi::error::check(MPI_Recv(&m, 1, MPI_INT, t_source, t_tag, t_comm, MPI_STATUS_IGNORE));
    mpi::error::check(MPI_Recv(&n, 1, MPI_INT, t_source, t_tag, t_comm, MPI_STATUS_IGNORE));
  }
};


class AckermannOutput : public mpi::Transmissable
{
public:
  unsigned ack = 0;

  AckermannOutput() : ack(0)
  {}

  AckermannOutput(unsigned const t_ack) : ack(t_ack)
  {}

  friend bool operator==(AckermannOutput const & lhs, AckermannOutput const & rhs)
  {
    return (lhs.ack == rhs.ack);
  }

  friend bool operator<(AckermannOutput const & lhs, AckermannOutput const & rhs)
  {
    return (lhs.ack < rhs.ack);
  }

  void mpiSend(int const t_destination, int const t_tag, MPI_Comm const t_comm) const
  {
    mpi::error::check(MPI_Send(&ack, 1, MPI_INT, t_destination, t_tag, t_comm));
  }

  void mpiReceive(int const t_source, int const t_tag, MPI_Comm const t_comm)
  {
    mpi::error::check(MPI_Recv(&ack, 1, MPI_INT, t_source, t_tag, t_comm, MPI_STATUS_IGNORE));
  }
};


// Test worker, computes ackermanns function
class AckermannWorker : public util::Worker<AckermannInput>
{
private:

  // Add member variables for output queues
  std::shared_ptr<util::Queue<AckermannOutput>> m_ackermann_result_queue;


  // Work to perform on each input
  virtual void processWorkItem(AckermannInput t_input)
  {
    this->m_ackermann_result_queue->enqueue(ack(t_input.m, t_input.n));
  }

public:
  AckermannWorker(
    std::shared_ptr<util::Queue<AckermannInput>> t_input_queue,
    std::shared_ptr<util::Queue<AckermannOutput>> t_ackermann_result_queue,
    unsigned const t_thread_count) :
    Worker(t_input_queue, t_thread_count),
    m_ackermann_result_queue(t_ackermann_result_queue)
  {}
};


SCENARIO(
  "Distributed compute test")
{
  unsigned queue_length = 16;

  int head_rank = 0;
  MPI_Comm communicator = MPI_COMM_WORLD;

  std::shared_ptr<util::Queue<AckermannInput>> input_queue(nullptr);
  std::shared_ptr<util::Queue<AckermannInput>> local_input_queue(new util::Queue<AckermannInput>(queue_length));

  std::shared_ptr<util::Queue<AckermannOutput>> local_output_queue(new util::Queue<AckermannOutput>(queue_length));
  std::shared_ptr<util::Queue<AckermannOutput>> output_queue(nullptr);

  if(mpi::comm::rank(communicator) == head_rank)
  {
    input_queue = std::shared_ptr<util::Queue<AckermannInput>>(new util::Queue<AckermannInput>(queue_length));
    output_queue = std::shared_ptr<util::Queue<AckermannOutput>>(new util::Queue<AckermannOutput>(queue_length));
  }

  GIVEN("A vector of {m, n}")
  {
    unsigned m_min = 2;
    unsigned m_max = 3;

    unsigned n_min = 8;
    unsigned n_max = 10;

    unsigned test_vector_length = 8192;

    std::vector<AckermannInput> input_vector(test_vector_length);
    std::vector<AckermannOutput> output_vector(test_vector_length);
    std::vector<AckermannOutput> expected_output(test_vector_length);

    unsigned ack_lookup[m_max + 1][n_max + 1];

    if(mpi::comm::rank(communicator) == head_rank)
    {
      for(unsigned m = 0; m <= m_max; m++)
      {
        for(unsigned n = 0; n <= n_max; n++)
        {
          ack_lookup[m][n] = ack(m, n);
        }
      }

      for(unsigned i = 0; i < test_vector_length; i++)
      {
        unsigned random_m = m_min + rand() % ((m_max - m_min) + 1);
        unsigned random_n = n_min + rand() % ((n_max - n_min) + 1);

        input_vector[i] = AckermannInput(random_m, random_n);

        expected_output[i] = AckermannOutput(ack_lookup[random_m][random_n]);
      }
    }

    WHEN("ack(m, n) is computed across multiple ranks using scatterer, worker & gatherer")
    {
      unsigned worker_threads = 12;

      util::Scatterer<AckermannInput> scatterer(input_queue, local_input_queue, communicator);
      util::Gatherer<AckermannOutput> gatherer(local_output_queue, output_queue, communicator);
      AckermannWorker worker(local_input_queue, local_output_queue, worker_threads);

      if(mpi::comm::rank(communicator) == head_rank)
      {
        std::thread enqueue_thread(&util::Queue<AckermannInput>::enqueueVector, &(*input_queue), std::ref(input_vector));
        std::thread dequeue_thread(&util::Queue<AckermannOutput>::dequeueVector, &(*output_queue), std::ref(output_vector));

        enqueue_thread.join();
        dequeue_thread.join();
      }

      THEN("Output values on the head rank are correct")
      {
        if(mpi::comm::rank(communicator) == head_rank)
        {
          std::sort(output_vector.begin(), output_vector.end());
          std::sort(expected_output.begin(), expected_output.end());

          bool vectors_match = (output_vector == expected_output);
          REQUIRE(vectors_match == true);
        }
      }
    }
  }
}
