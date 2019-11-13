#define CATCH_CONFIG_RUNNER
#include "catch.hpp"


// External
#include "mpi.h"


// Distributed tests require that MPI is initialised, so lets do that
int main(int argc, char** argv)
{
  MPI_Init(&argc, &argv);

  // Run catch tests
  int result = Catch::Session().run(argc, argv);

  MPI_Finalize();
  return result;
}
