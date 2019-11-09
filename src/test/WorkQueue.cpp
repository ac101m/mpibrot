// This is a catch module
#include "catch.hpp"


// Standard
#include <vector>
#include <iostream>
#include "stdlib.h"
#include "time.h"

// Internal
#include "util/WorkQueue.hpp"


// Ackermanns function (yuck)
unsigned ack(unsigned m, unsigned n)
{
  if (m == 0) return n + 1;
  if (n == 0) return ack(m - 1, 1);
  return ack(m - 1, ack(m, n - 1));
}


class AckermannWorkItem : public util::WorkItem
{
private:
  unsigned m_m, m_n;
  unsigned m_ackermann;

public:
  AckermannWorkItem(unsigned t_m_initial, unsigned t_n_initial) :
    m_m(t_m_initial), m_n(t_n_initial)
  {}

  unsigned getValue()
  {
    return m_ackermann;
  }

  void execute()
  {
    m_ackermann = ack(m_m, m_n);
  }
};


SCENARIO(
  "[Compute engine] - Ackermann work queue test")
{
  GIVEN("A vector of work items")
  {
    unsigned test_vector_size = 4096;

    srand(time(NULL));

    std::vector<std::shared_ptr<AckermannWorkItem>> input_work_items(test_vector_size);
    std::vector<std::shared_ptr<AckermannWorkItem>> output_work_items(test_vector_size);

    std::vector<unsigned> output(test_vector_size, 0);
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

      expected_output[i] = ack_values[random_m][random_n];

      input_work_items[i] = std::shared_ptr<AckermannWorkItem>(new AckermannWorkItem(random_m, random_n));
    }

    WHEN("They are processed by a work queue")
    {
      unsigned thread_count = 24;
      unsigned input_buffer_size = 4;
      unsigned output_buffer_size = 4;

      util::WorkQueue<AckermannWorkItem> work_queue(thread_count, input_buffer_size, output_buffer_size);

      std::thread enqueue_thread(&util::WorkQueue<AckermannWorkItem>::enqueueVector, &work_queue, std::ref(input_work_items));
      std::thread dequeue_thread(&util::WorkQueue<AckermannWorkItem>::dequeueVector, &work_queue, std::ref(output_work_items));

      enqueue_thread.join();
      dequeue_thread.join();

      for(unsigned i = 0; i < output_work_items.size(); i++)
      {
        output[i] = output_work_items[i]->getValue();
      }

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
