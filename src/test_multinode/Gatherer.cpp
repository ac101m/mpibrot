// This is a catch module
#include "catch.hpp"


// Internal
#include "test_multinode/TransmissableInt.hpp"
#include "util/Gatherer.hpp"
#include "util/Queue.hpp"

// Standard
#include <memory>


SCENARIO(
  "[Gatherer] - Single rank test")
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

  GIVEN("A gatherer with one transmit and one receive thread")
  {
    unsigned head_node = 0;
    unsigned tx_threads = 1;
    unsigned rx_threads = 1;

    util::Gatherer<TransmissableInt> gatherer(input_queue, output_queue, MPI_COMM_SELF, head_node, tx_threads, rx_threads);

    WHEN("A vector of transmissable items is passed through the gatherer")
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

  GIVEN("A gatherer with multiple transmit and receive threads")
  {
    unsigned head_node = 0;
    unsigned tx_threads = 4;
    unsigned rx_threads = 4;

    util::Gatherer<TransmissableInt> gatherer(input_queue, output_queue, MPI_COMM_SELF, head_node, tx_threads, rx_threads);

    WHEN("A vector of transmissable items is passed through the gatherer")
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
  "[Gatherer] - Collective test")
{
  unsigned input_queue_length = 4;
  unsigned output_queue_length = 4;

  int head_node = 0;
  MPI_Comm communicator = MPI_COMM_WORLD;

  std::shared_ptr<util::Queue<TransmissableInt>> input_queue(new util::Queue<TransmissableInt>(input_queue_length));
  std::shared_ptr<util::Queue<TransmissableInt>> output_queue(nullptr);


  unsigned test_vector_length = 64;

  std::vector<TransmissableInt> input_vector = std::vector<TransmissableInt>(test_vector_length);
  std::vector<TransmissableInt> output_vector = std::vector<TransmissableInt>(test_vector_length * mpi::comm::size(communicator));
  std::vector<TransmissableInt> expected_output = std::vector<TransmissableInt>(test_vector_length * mpi::comm::size(communicator));

  for(unsigned i = 0; i < input_vector.size(); i++)
  {
    input_vector[i] = (mpi::comm::rank(communicator) * input_vector.size()) + i;
  }

  for(unsigned i = 0; i < expected_output.size(); i++)
  {
    expected_output[i] = i;
  }

  GIVEN("A gatherer with one transmit and one receive thread")
  {
    unsigned tx_threads = 1;
    unsigned rx_threads = 1;

    if(mpi::comm::rank(communicator) == head_node)
    {
      output_queue = std::shared_ptr<util::Queue<TransmissableInt>>(new util::Queue<TransmissableInt>(output_queue_length));
    }

    util::Gatherer<TransmissableInt> gatherer(input_queue, output_queue, communicator, head_node, tx_threads, rx_threads);

    WHEN("Data is enqueued on all ranks, and dequeued one one rank")
    {
      std::thread enqueue_thread(&util::Queue<TransmissableInt>::enqueueVector, &(*input_queue), std::ref(input_vector));

      if(mpi::comm::rank(communicator) == head_node)
      {
        std::thread dequeue_thread(&util::Queue<TransmissableInt>::dequeueVector, &(*output_queue), std::ref(output_vector));
        dequeue_thread.join();
      }

      enqueue_thread.join();

      THEN("Output matches expected output (on the head node)")
      {
        if(mpi::comm::rank(communicator) == head_node)
        {
          std::sort(expected_output.begin(), expected_output.end());
          std::sort(output_vector.begin(), output_vector.end());

          bool vectors_match = (output_vector == expected_output);
          REQUIRE(vectors_match == true);
        }
      }
    }
  }

  GIVEN("A gatherer with multiple transmit and receive threads")
  {
    unsigned tx_threads = 4;
    unsigned rx_threads = 4;

    if(mpi::comm::rank(communicator) == head_node)
    {
      output_queue = std::shared_ptr<util::Queue<TransmissableInt>>(new util::Queue<TransmissableInt>(output_queue_length));
    }

    util::Gatherer<TransmissableInt> gatherer(input_queue, output_queue, communicator, head_node, tx_threads, rx_threads);

    WHEN("Data is enqueued on all ranks, and dequeued one one rank")
    {
      std::thread enqueue_thread(&util::Queue<TransmissableInt>::enqueueVector, &(*input_queue), std::ref(input_vector));

      if(mpi::comm::rank(communicator) == head_node)
      {
        std::thread dequeue_thread(&util::Queue<TransmissableInt>::dequeueVector, &(*output_queue), std::ref(output_vector));
        dequeue_thread.join();
      }

      enqueue_thread.join();

      THEN("Output matches expected output (on the head node)")
      {
        if(mpi::comm::rank(communicator) == head_node)
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
