// This is a catch module
#include "catch.hpp"


// Standard
#include <thread>

// Internal
#include "util/CountingSemaphore.hpp"


// Give a semaphore a bunch of times
void giveSemaphore(
  util::CountingSemaphore * const t_semaphore,
  unsigned const t_give_count)
{
  for(unsigned i = 0; i < t_give_count; i++)
  {
    t_semaphore->give();
  }
}


// Take a semaphore a bunch of times
// Will block if count is not high enough
void takeSemaphore(
  util::CountingSemaphore * const t_semaphore,
  unsigned const t_take_count)
{
  for(unsigned i = 0; i < t_take_count; i++)
  {
    t_semaphore->take();
  }
}


// Take a semaphore a bunch of times, return as soon as it fails
void tryTakeSemaphore(
  util::CountingSemaphore * const t_semaphore,
  unsigned * const t_take_count)
{
  *t_take_count = 0;
  while(t_semaphore->tryTake()) {
    (*t_take_count)++;
  }
}


SCENARIO(
  "Single thread counting semaphore test",
  "[CountingSemaphore]")
{
  GIVEN("A counting semaphore with an initial count of zero")
  {
    util::CountingSemaphore semaphore;

    WHEN("The count is retrieved having performed no other operations")
    {
      THEN("The retrieved count is zero")
      {
        REQUIRE(semaphore.count() == 0);
      }
    }

    WHEN("An attempt is made to take the semaphore having performed no other operations")
    {
      THEN("The operation fails")
      {
        REQUIRE(semaphore.tryTake() == false);
      }
    }

    WHEN("The semaphore is given n times")
    {
      unsigned n = 16;

      for(unsigned i = 0; i < n; i++)
      {
        semaphore.give();
      }

      THEN("The semaphore count is n")
      {
        REQUIRE(semaphore.count() == n);
      }
    }
  }

  GIVEN("A counting semaphore with an initial count of n")
  {
    unsigned n = 16;

    util::CountingSemaphore semaphore(n);

    WHEN("The count is retrieved having performed no other operations")
    {
      THEN("The retrieved count is n")
      {
        REQUIRE(semaphore.count() == n);
      }
    }

    WHEN("The semaphore is taken repeatedly")
    {
      unsigned take_success_count = 0;

      tryTakeSemaphore(&semaphore, &take_success_count);

      THEN("The semaphore can be taken n times")
      {
        REQUIRE(take_success_count == n);
      }
    }
  }
}


SCENARIO(
  "Multi-thread counting semaphore test",
  "[CountingSemaphore]")
{
  GIVEN("A counting semaphore with an initial count of zero")
  {
    util::CountingSemaphore semaphore;

    WHEN("Multiple threads give the semaphore simultaneously")
    {
      unsigned thread_count = 4;
      unsigned gives_per_thread = 65536;

      std::vector<std::unique_ptr<std::thread>> give_threads(thread_count);

      for(unsigned i = 0; i < give_threads.size(); i++)
      {
        give_threads[i] = std::unique_ptr<std::thread>(
          new std::thread(giveSemaphore, &semaphore, gives_per_thread));
      }

      for(unsigned i = 0; i < give_threads.size(); i++)
      {
        give_threads[i]->join();
      }

      THEN("The semaphore count is correctly preserved")
      {
        REQUIRE(semaphore.count() == (thread_count * gives_per_thread));
      }
    }

    WHEN("Give and take are called concurrently from two threads")
    {
      unsigned take_count = 32768;
      unsigned give_count = 65536;

      std::thread take_thread(takeSemaphore, &semaphore, take_count);
      std::thread give_thread(giveSemaphore, &semaphore, give_count);

      give_thread.join();
      take_thread.join();

      THEN("The semaphore count is correctly preserved")
      {
        REQUIRE(semaphore.count() == (give_count - take_count));
      }
    }
  }

  GIVEN("A counting semaphore with a non-zero initial count")
  {
    unsigned thread_count = 4;
    unsigned takes_per_thread = 65536;
    unsigned initial_count = thread_count * takes_per_thread * 2;

    util::CountingSemaphore semaphore(initial_count);

    WHEN("Multiple threads take the semaphore simultaneously")
    {
      std::vector<std::unique_ptr<std::thread>> take_threads(thread_count);

      for(unsigned i = 0; i < take_threads.size(); i++)
      {
        take_threads[i] = std::unique_ptr<std::thread>(
          new std::thread(takeSemaphore, &semaphore, takes_per_thread));
      }

      for(unsigned i = 0; i < take_threads.size(); i++)
      {
        take_threads[i]->join();
      }

      THEN("The semaphore count is correctly preserved")
      {
        REQUIRE(semaphore.count() == initial_count - (thread_count * takes_per_thread));
      }
    }
  }
}
