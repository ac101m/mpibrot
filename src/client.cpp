
// Internal
#include "util/Options.hpp"
#include "draw/Image.hpp"

// External
#include "GLT/Window.hpp"

// Standard
#include <vector>
#include <iostream>


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
  window.MakeCurrent();

  // Load shaders
  GLT::ShaderProgram shader({
    GLT::Shader(GL_VERTEX_SHADER, "data/shaders/vs.glsl"),
    GLT::Shader(GL_FRAGMENT_SHADER, "data/shaders/fs.glsl")});

  // Create orbit texture
  Image image(wSize[0], wSize[1]);

  // Draw loop
  while(!window.ShouldClose()) {
    image.Random();
    image.Update();
    image.Draw(shader);
    window.Refresh();
  }

  // El fin
  return 0;
}
