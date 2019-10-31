// This is a catch module
#include "catch.hpp"


// Standard
#include <vector>
#include <thread>
#include <algorithm>
#include <stdlib.h>
#include <time.h>

// Internal
#include "util/SyncQueue.hpp"


std::vector<int> genTestVector(
  unsigned const t_size,
  unsigned const t_max = RAND_MAX)
{
  std::vector<int> test_vector(t_size);

  for(unsigned i = 0; i < test_vector.size(); i++)
  {
    test_vector[i] = rand() % (t_max + 1);
  }

  return test_vector;
}


SCENARIO(
  "[Threaded queue] - Serial read write test")
{
  GIVEN("An arbitrary variable")
  {
    float enqueue_value = 3.14159;

    WHEN("The variable is enqueued, then dequeued")
    {
      unsigned queue_length = 32;

      util::SyncQueue<float> queue(queue_length);

      queue.enqueue(enqueue_value);

      float dequeue_value = queue.dequeue();

      THEN("It's value does not change")
      {
        REQUIRE(enqueue_value == dequeue_value);
      }
    }
  }

  GIVEN("An arbitrary vector of variables for which length == queue capacity")
  {
    unsigned queue_length = 32;
    unsigned test_vector_length = 32;

    std::vector<int> input_vector = genTestVector(test_vector_length);
    std::vector<int> output_vector(test_vector_length);

    util::SyncQueue<int> queue(queue_length);

    WHEN("The vector is enqueued, then dequeued")
    {
      queue.enqueueVector(input_vector);
      queue.dequeueVector(output_vector);

      THEN("The vectors values are preserved")
      {
        bool vectors_match = (input_vector == output_vector);
        REQUIRE(vectors_match == true);
      }
    }
  }
}


SCENARIO(
  "[Synchronized queue] - Asynchronous serial read-write test")
{
  GIVEN("An arbitrary vector of variables")
  {
    unsigned test_vector_length = 65536;

    std::vector<int> input_vector = genTestVector(test_vector_length);
    std::vector<int> output_vector(test_vector_length);

    WHEN("The vector is queued and dequeued concurrently in separate threads")
    {
      unsigned queue_length = 32;

      util::SyncQueue<int> queue(queue_length);

      std::thread enqueue_thread(&util::SyncQueue<int>::enqueueVector, &queue, std::ref(input_vector));
      std::thread dequeue_thread(&util::SyncQueue<int>::dequeueVector, &queue, std::ref(output_vector));

      enqueue_thread.join();
      dequeue_thread.join();

      THEN("Vector values are correctly preserved")
      {
        bool vectors_match = (input_vector == output_vector);
        REQUIRE(vectors_match == true);
      }
    }
  }
}


SCENARIO(
  "[Synchronized  queue] - Asynchronous parallel read/write test")
{
  GIVEN("An arbitrary vector of variables")
  {
    unsigned test_vector_length = 262144;

    std::vector<int> input_vector = genTestVector(test_vector_length);

    WHEN("The vector is queued and dequeued concurrently by many threads")
    {
      unsigned thread_count = 4;
      unsigned queue_length = 32;

      std::vector<std::vector<int>> enqueue_vectors(thread_count);
      std::vector<std::vector<int>> dequeue_vectors(thread_count);

      for(unsigned i = 0; i < input_vector.size(); i++)
      {
        enqueue_vectors[i % thread_count].push_back(input_vector[i]);
        dequeue_vectors[i % thread_count].push_back(0);
      }

      util::SyncQueue<int> queue(queue_length);

      std::vector<std::thread> enqueue_threads(thread_count);
      std::vector<std::thread> dequeue_threads(thread_count);

      for(unsigned i = 0; i < thread_count; i++)
      {
        enqueue_threads[i] = std::thread(&util::SyncQueue<int>::enqueueVector, &queue, std::ref(enqueue_vectors[i]));
        dequeue_threads[i] = std::thread(&util::SyncQueue<int>::dequeueVector, &queue, std::ref(dequeue_vectors[i]));
      }

      for(unsigned i = 0; i < enqueue_threads.size(); i++)
      {
        enqueue_threads[i].join();
      }

      for(unsigned i = 0; i < dequeue_threads.size(); i++)
      {
        dequeue_threads[i].join();
      }

      std::vector<int> output_vector;

      for(unsigned i = 0; i < dequeue_vectors.size(); i++)
      {
        output_vector.insert(output_vector.end(), dequeue_vectors[i].begin(), dequeue_vectors[i].end());
      }

      THEN("Vector values are preserved")
      {
        std::sort(input_vector.begin(), input_vector.end());
        std::sort(output_vector.begin(), output_vector.end());

        bool vectors_match = (input_vector == output_vector);
        REQUIRE(vectors_match == true);
      }
    }
  }
}
