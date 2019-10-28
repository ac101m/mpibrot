// Standard
#include <iostream>

// External
#include "mpi.h"

// Internal
#include "util/Options.hpp"


int main(int argc, char** argv) {
  OptionParser opt(GenServerOptionParser(argc, argv));
  std::cout << "Hello world\n";
  return 0;
}