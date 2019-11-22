// This is a catch module
#include "catch.hpp"


// Internal
#include "test_multinode/TransmissableInt.hpp"
#include "util/Distributor.hpp"

// Standard
#include <vector>
#include <memory>
#include <algorithm>


SCENARIO(
  "[Distributor] - Single rank test")
{
  unsigned input_queue_length = 4;
  unsigned output_queue_length = 4;

  std::shared_ptr<util::Queue<TransmissableInt>> input_queue(new util::Queue<TransmissableInt>(input_queue_length));
  std::shared_ptr<util::Queue<TransmissableInt>> output_queue(new util::Queue<TransmissableInt>(output_queue_length));

  unsigned test_vector_length = 1024;

  std::vector<TransmissableInt> input_vector = std::vector<TransmissableInt>(test_vector_length);
  std::vector<TransmissableInt> output_vector = std::vector<TransmissableInt>(test_vector_length);

  for(unsigned i = 0; i < test_vector_length; i++)
  {
    input_vector[i] = rand();
  }

  GIVEN("A distributor with one send and receive thread")
  {
    unsigned signal_groups = 1;
    unsigned tx_threads = 1;
    unsigned signal_threads = 1;

    util::Distributor<TransmissableInt> distributor(input_queue, output_queue, MPI_COMM_SELF, signal_groups, tx_threads, signal_threads);

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

  GIVEN("A distributor with multiple send and receive thread")
  {
    unsigned signal_groups = 1;
    unsigned tx_threads = 4;
    unsigned signal_threads = 4;

    util::Distributor<TransmissableInt> distributor(input_queue, output_queue, MPI_COMM_SELF, signal_groups, tx_threads, signal_threads);

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
