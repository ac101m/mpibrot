
// Standard
#include <vector>
#include <iostream>
#include "unistd.h"

// Internal
#include "util/Options.hpp"

// External
#include "GLT/Window.hpp"


// Lets get this show on the road
int main(int argc, char **argv) {
  OptionParser opt = GenOptionParser(argc, argv);

  // Get window dimensions
  std::vector<int> wSize = opt.Get("window-resolution");
  if(wSize.size() < 2) {
    std::cerr << "ERROR, Please specify both window width and height\n";
    exit(1);
  }

  // Open a window with specified dimensions
  GLT::Window window = GLT::Window(wSize[0], wSize[1], "mpibrot");

  // Draw loop
  while(!window.ShouldClose()) {
    window.Refresh();
  }

  // El fin
  return 0;
}
