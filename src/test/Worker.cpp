// This is a catch module
#include "catch.hpp"


// Standard
#include <vector>
#include <iostream>
#include <memory>
#include "stdlib.h"
#include "time.h"

// Internal
#include "util/Worker.hpp"


class AckermannInput
{
public:
  bool stop = true;
  unsigned m = 0;
  unsigned n = 0;

  friend bool operator!=(AckermannInput const& lhs, AckermannInput const& rhs)
  {
    return (lhs.m != rhs.m) || (lhs.n != rhs.n) || (lhs.stop != rhs.stop);
  }
};


// Ackermanns function (yuck)
unsigned ack(unsigned m, unsigned n)
{
  if (m == 0) return n + 1;
  if (n == 0) return ack(m - 1, 1);
  return ack(m - 1, ack(m, n - 1));
}


// Test worker, computes ackermanns function
class AckermannWorker : public util::Worker<AckermannInput>
{
private:

  // Add member variables for output queues
  std::shared_ptr<util::Queue<unsigned>> m_ackermann_result_queue;


  // Work to perform on each input
  virtual void processWorkItem(AckermannInput t_input)
  {
    this->m_ackermann_result_queue->enqueue(ack(t_input.m, t_input.n));
  }

public:
  AckermannWorker(
    std::shared_ptr<util::Queue<AckermannInput>> t_input_queue,
    std::shared_ptr<util::Queue<unsigned>> t_ackermann_result_queue,
    unsigned const t_thread_count) :
    Worker(t_input_queue, t_thread_count),
    m_ackermann_result_queue(t_ackermann_result_queue)
  {}
};


SCENARIO(
  "[Compute engine] - Ackermann work queue test")
{
  GIVEN("A vector of work items")
  {
    unsigned test_vector_size = 4096;

    srand(time(NULL));

    std::vector<AckermannInput> input(test_vector_size);
    std::vector<unsigned> output(test_vector_size);

    std::vector<unsigned> expected_output(test_vector_size);

    unsigned m_max = 3;
    unsigned n_max = 10;

    unsigned ack_values[m_max + 1][n_max + 1];

    for(unsigned m = 0; m <= m_max; m++)
    {
      for(unsigned n = 0; n <= n_max; n++)
      {
        ack_values[m][n] = ack(m, n);
      }
    }

    for(unsigned i = 0; i < test_vector_size; i++)
    {
      unsigned random_m = rand() % (m_max + 1);
      unsigned random_n = rand() % (n_max + 1);

      input[i] = {false, random_m, random_n};

      expected_output[i] = ack_values[random_m][random_n];
    }

    WHEN("They are processed by a work queue")
    {
      unsigned thread_count = 24;
      unsigned input_queue_size = 4;
      unsigned output_queue_size = 4;

      std::shared_ptr<util::Queue<AckermannInput>> input_queue(new util::Queue<AckermannInput>(input_queue_size));
      std::shared_ptr<util::Queue<unsigned>> output_queue(new util::Queue<unsigned>(output_queue_size));

      AckermannWorker worker(input_queue, output_queue, thread_count);

      std::thread enqueue_thread(&util::Queue<AckermannInput>::enqueueVector, &(*input_queue), std::ref(input));
      std::thread dequeue_thread(&util::Queue<unsigned>::dequeueVector, &(*output_queue), std::ref(output));

      enqueue_thread.join();
      dequeue_thread.join();

      THEN("The results match those achieved when processed serially")
      {
        std::sort(output.begin(), output.end());
        std::sort(expected_output.begin(), expected_output.end());

        bool vectors_match = (output == expected_output);
        REQUIRE(vectors_match == true);
      }
    }
  }
}
