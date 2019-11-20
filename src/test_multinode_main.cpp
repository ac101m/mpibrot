#define CATCH_CONFIG_RUNNER
#include "catch.hpp"


// External
#include "mpi.h"

// Internal
#include "mpi/error.hpp"


// Distributed tests require that MPI is initialised, so lets do that
int main(int argc, char** argv)
{
  int thread_level_required = MPI_THREAD_MULTIPLE;
  int thread_level_actual;

  mpi::error::check(MPI_Init_thread(&argc, &argv, thread_level_required, &thread_level_actual));

  if(thread_level_actual != thread_level_required)
  {
    std::cout << "Error, could not initialise threaded MPI environment\n";
    exit(1);
  }

  // Run catch tests
  int result = Catch::Session().run(argc, argv);

  mpi::error::check(MPI_Finalize());
  return result;
}
