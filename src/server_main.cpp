// Internal

// External
#include "optparse.hpp"
#include "mpi.h"

// Standard
#include <iostream>


// Server options
OptionParser genOptionParser(int argc, char** argv)
{
  OptionParser opt(
    argc, argv,
    "Parallel mandelbrot zoom generator for clusters - server");

  opt.Add(Option(
    "port", 'p', ARG_TYPE_INT,
    "Port for server to listen on",
    {"9901"}));

  return opt;
}


int main(int argc, char** argv)
{
  OptionParser opt = genOptionParser(argc, argv);
  std::cout << "Hello world\n";
  return 0;
}
