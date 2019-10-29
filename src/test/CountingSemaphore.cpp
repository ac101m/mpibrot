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


// Take a semaphore a bunch of times, return as soon as it fails
void tryTakeSemaphore(
  util::CountingSemaphore * const t_semaphore,
  unsigned * const t_take_count)
{
  *t_take_count = 0;
  while(t_semaphore->try_take()) {
    (*t_take_count)++;
  }
}


TEST_CASE("Initialisation take test", "[counting semaphore]")
{
  unsigned initial_count = 64;
  unsigned take_count = 0;

  util::CountingSemaphore semaphore(initial_count);

  tryTakeSemaphore(&semaphore, &take_count);

  REQUIRE(take_count == initial_count);
}


TEST_CASE("Serial take test", "[counting semaphore]")
{
  util::CountingSemaphore semaphore;

  unsigned give_count = 64;
  unsigned take_count = 0;

  giveSemaphore(&semaphore, give_count);
  tryTakeSemaphore(&semaphore, &take_count);

  REQUIRE(take_count == give_count);
}


TEST_CASE("Parallel take test", "[counting semaphore]")
{
  util::CountingSemaphore semaphore;

  unsigned give_count = 8192;
  unsigned take_count = 0;
  unsigned num_threads = 4;

  giveSemaphore(&semaphore, give_count);

  std::vector<unsigned> take_counts(num_threads, 0);
  std::vector<std::unique_ptr<std::thread>> take_threads(num_threads);

  for(unsigned i = 0; i < num_threads; i++)
  {
    take_threads[i] = std::unique_ptr<std::thread>(
      new std::thread(tryTakeSemaphore, &semaphore, &take_counts[i]));
  }

  for(unsigned i = 0; i < num_threads; i++)
  {
    take_threads[i]->join();
    take_count += take_counts[i];
  }

  REQUIRE(take_count == give_count);
}
