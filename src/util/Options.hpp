#ifndef MPIBROT_OPTIONS_INCLUDED
#define MPIBROT_OPTIONS_INCLUDED

// External
#include "optparse.hpp"


// Create option parser
OptionParser GenOptionParser(int argc, char** argv) {
  OptionParser opt(argc, argv, "Parallel mandelbrot zoom generator for clusters");

  // Viewing window resolution option
  opt.Add(Option(
    "window-resolution", 'w', ARG_TYPE_INT,
    "Screen resolution for window",
    {"1024", "768"}));

  // Return the constructed option parser
  return opt;
}


#endif // MPIBROT_OPTIONS_INCLUDED
