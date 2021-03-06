// This is a catch module
#include "catch.hpp"


// Internal
#include "test_multinode/TransmissableInt.hpp"
#include "util/Scatterer.hpp"
#include "util/Gatherer.hpp"
#include "util/Queue.hpp"

// Standard
#include <memory>


SCENARIO(
  "[Scatterer] - Single rank test")
{
  unsigned input_queue_length = 4;
  unsigned output_queue_length = 4;

  int head_node = 0;
  MPI_Comm communicator = MPI_COMM_SELF;

  std::shared_ptr<util::Queue<TransmissableInt>> input_queue(new util::Queue<TransmissableInt>(input_queue_length));
  std::shared_ptr<util::Queue<TransmissableInt>> output_queue(new util::Queue<TransmissableInt>(output_queue_length));

  unsigned test_vector_length = 1024;

  std::vector<TransmissableInt> input_vector = std::vector<TransmissableInt>(test_vector_length);
  std::vector<TransmissableInt> output_vector = std::vector<TransmissableInt>(test_vector_length);

  for(unsigned i = 0; i < test_vector_length; i++)
  {
    input_vector[i] = rand();
  }

  GIVEN("A scatterer with one send and one receive thread")
  {
    unsigned tx_threads = 1;
    unsigned rx_threads = 1;

    util::Scatterer<TransmissableInt> scatterer(input_queue, output_queue, communicator, head_node, tx_threads, rx_threads);

    WHEN("A vector of tranmissable items is passed through the scatterer")
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

  GIVEN("A scatterer with multiple send and receive threads")
  {
    unsigned tx_threads = 4;
    unsigned rx_threads = 4;

    util::Scatterer<TransmissableInt> scatterer(input_queue, output_queue, communicator, head_node, tx_threads, rx_threads);

    WHEN("A vector of transmissable items is passed through the scatterer")
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
  "[Scatterer] - Collective test")
{
  unsigned input_queue_length = 4;
  unsigned intermediate_queue_length = 4;
  unsigned output_queue_length = 4;

  int head_node = 0;
  MPI_Comm communicator = MPI_COMM_WORLD;

  std::shared_ptr<util::Queue<TransmissableInt>> input_queue(nullptr);
  std::shared_ptr<util::Queue<TransmissableInt>> intermediate_queue(new util::Queue<TransmissableInt>(intermediate_queue_length));
  std::shared_ptr<util::Queue<TransmissableInt>> output_queue(nullptr);

  if(mpi::comm::rank(communicator) == head_node)
  {
    input_queue = std::shared_ptr<util::Queue<TransmissableInt>>(new util::Queue<TransmissableInt>(input_queue_length));
    output_queue = std::shared_ptr<util::Queue<TransmissableInt>>(new util::Queue<TransmissableInt>(output_queue_length));
  }

  unsigned test_vector_length = 1024;

  std::vector<TransmissableInt> input_vector = std::vector<TransmissableInt>(test_vector_length);
  std::vector<TransmissableInt> output_vector = std::vector<TransmissableInt>(test_vector_length);

  for(unsigned i = 0; i < test_vector_length; i++)
  {
    input_vector[i] = rand();
  }

  GIVEN("A scatterer with one send and one receive thread")
  {
    unsigned tx_threads = 1;
    unsigned rx_threads = 1;

    util::Scatterer<TransmissableInt> scatterer(input_queue, intermediate_queue, communicator, head_node, tx_threads, rx_threads);
    util::Gatherer<TransmissableInt> gatherer(intermediate_queue, output_queue, communicator, head_node);

    WHEN("A vector of transmissable items is passed through the scatterer")
    {

      if(mpi::comm::rank(communicator) == head_node)
      {
        std::thread enqueue_thread(&util::Queue<TransmissableInt>::enqueueVector, &(*input_queue), std::ref(input_vector));
        std::thread dequeue_thread(&util::Queue<TransmissableInt>::dequeueVector, &(*output_queue), std::ref(output_vector));

        enqueue_thread.join();
        dequeue_thread.join();
      }

      THEN("The values are preserved")
      {
        if(mpi::comm::rank(communicator) == head_node)
        {
          std::sort(input_vector.begin(), input_vector.end());
          std::sort(output_vector.begin(), output_vector.end());

          bool vectors_match = (input_vector == output_vector);
          REQUIRE(vectors_match == true);
        }
      }
    }
  }

  GIVEN("A scatterer with multiple send and receive threads")
  {
    unsigned tx_threads = 4;
    unsigned rx_threads = 4;

    util::Scatterer<TransmissableInt> scatterer(input_queue, intermediate_queue, MPI_COMM_WORLD, head_node, tx_threads, rx_threads);
    util::Gatherer<TransmissableInt> gatherer(intermediate_queue, output_queue, MPI_COMM_WORLD, head_node);

    WHEN("A vector of transmissable items is passed through the scatterer")
    {
      if(mpi::comm::rank(communicator) == head_node)
      {
        std::thread enqueue_thread(&util::Queue<TransmissableInt>::enqueueVector, &(*input_queue), std::ref(input_vector));
        std::thread dequeue_thread(&util::Queue<TransmissableInt>::dequeueVector, &(*output_queue), std::ref(output_vector));

        enqueue_thread.join();
        dequeue_thread.join();
      }

      THEN("The values are preserved")
      {
        if(mpi::comm::rank(communicator) == head_node)
        {
          std::sort(input_vector.begin(), input_vector.end());
          std::sort(output_vector.begin(), output_vector.end());

          bool vectors_match = (input_vector == output_vector);
          REQUIRE(vectors_match == true);
        }
      }
    }
  }
}
