// This is a catch module
#include "catch.hpp"


// Internal
#include "test_multinode/TransmissableInt.hpp"
#include "util/Distributor.hpp"
#include "util/Gatherer.hpp"

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

  GIVEN("A distributor with one send and one receive thread")
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

  GIVEN("A distributor with multiple send and receive threads")
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


SCENARIO(
  "[Distributor] - Collective test")
{
  unsigned input_queue_length = 4;
  unsigned intermediate_queue_length = 4;
  unsigned output_queue_length = 4;

  int head_rank = 0;
  MPI_Comm communicator = MPI_COMM_WORLD;

  std::shared_ptr<util::Queue<TransmissableInt>> input_queue(new util::Queue<TransmissableInt>(input_queue_length));
  std::shared_ptr<util::Queue<TransmissableInt>> intermediate_queue(new util::Queue<TransmissableInt>(intermediate_queue_length));
  std::shared_ptr<util::Queue<TransmissableInt>> output_queue(nullptr);

  if(mpi::comm::rank(communicator) == head_rank)
  {
    output_queue = std::shared_ptr<util::Queue<TransmissableInt>>(new util::Queue<TransmissableInt>(output_queue_length));
  }

  unsigned test_vector_length = 64;

  std::vector<TransmissableInt> input_vector(test_vector_length);
  std::vector<TransmissableInt> output_vector(test_vector_length * mpi::comm::size(communicator));
  std::vector<TransmissableInt> expected_output(test_vector_length * mpi::comm::size(communicator));

  for(unsigned i = 0; i < input_vector.size(); i++)
  {
    input_vector[i] = (mpi::comm::rank(communicator) * input_vector.size()) + i;
  }

  for(unsigned i = 0; i < expected_output.size(); i++)
  {
    expected_output[i] = i;
  }

  GIVEN("A distributor with a single signal handler and transmit thread")
  {
    unsigned signal_group_count = 1;
    unsigned transmit_thread_count = 1;
    unsigned signal_handler_thread_count = 1;

    util::Distributor<TransmissableInt> distributor(input_queue, intermediate_queue, communicator, signal_group_count, transmit_thread_count, signal_handler_thread_count);
    util::Gatherer<TransmissableInt> gatherer(intermediate_queue, output_queue, communicator);

    WHEN("Data is passed through the distributor on all nodes")
    {
      std::thread enqueue_thread(&util::Queue<TransmissableInt>::enqueueVector, &(*input_queue), std::ref(input_vector));

      if(mpi::comm::rank(communicator) == head_rank)
      {
        std::thread dequeue_thread(&util::Queue<TransmissableInt>::dequeueVector, &(*output_queue), std::ref(output_vector));
        dequeue_thread.join();
      }

      enqueue_thread.join();

      THEN("Values are preserved")
      {
        if(mpi::comm::rank(communicator) == head_rank)
        {
          std::sort(expected_output.begin(), expected_output.end());
          std::sort(output_vector.begin(), output_vector.end());

          bool vectors_match = (output_vector == expected_output);
          REQUIRE(vectors_match == true);
        }
      }
    }
  }
}
