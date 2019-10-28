#ifndef MPIBROT_OPTIONS_INCLUDED
#define MPIBROT_OPTIONS_INCLUDED

// External
#include "optparse.hpp"


// Client options
OptionParser genClientOptionParser(int argc, char** argv) {
  OptionParser opt(
    argc, argv,
    "Parallel mandelbrot zoom generator for clusters - client");

  // Viewing window resolution option
  opt.Add(Option(
    "window-resolution", 'w', ARG_TYPE_INT,
    "Screen resolution for window",
    {"1024", "768"}));

  return opt;
}


// Server options
OptionParser genServerOptionParser(int argc, char** argv) {
  OptionParser opt(
    argc, argv,
    "Parallel mandelbrot zoom generator for clusters - server");

  // Viewing window resolution option
  opt.Add(Option(
    "port", 'p', ARG_TYPE_INT,
    "Port to host server on",
    {"978"}));

  return opt;
}


#endif // MPIBROT_OPTIONS_INCLUDED
