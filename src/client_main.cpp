
// Internal
#include "draw/Image.hpp"
#include "compute/SimpleBrot.hpp"

// External
#include "optparse.hpp"
#include "GLT/Window.hpp"

// Standard
#include <vector>
#include <iostream>


// Generate option parser
OptionParser genOptionParser(int argc, char** argv)
{
  OptionParser opt(
    argc, argv,
    "Parallel mandelbrot zoom generator for clusters - client");

  // Viewing window resolution option
  opt.Add(Option(
    "window-resolution", 'w', ARG_TYPE_INT,
    "Screen resolution for window",
    {"1024", "768"}));

  opt.Add(Option(
    "port", 'p', ARG_TYPE_INT,
    "Port for server connection",
    {"9901"}));

  opt.Add(Option(
    "host", 'H', ARG_TYPE_STRING,
    "Address of host compute server",
    {"127.0.0.1"}));

  return opt;
}


// Lets get this show on the road
int main(int argc, char **argv)
{
  OptionParser opt = genOptionParser(argc, argv);

  // Get window dimensions
  std::vector<int> wSize = opt.Get("window-resolution");
  if(wSize.size() < 2)
  {
    std::cerr << "ERROR, Please specify both window width and height\n";
    exit(1);
  }

  // Rename window size to be less annoying to work with
  int& width = wSize[0];
  int& height = wSize[1];

  // Open a window with specified dimensions
  GLT::Window window = GLT::Window(width, height, "mpibrot");
  window.MakeCurrent();

  // Load shaders
  GLT::ShaderProgram shader({
    GLT::Shader(GL_VERTEX_SHADER, "data/shaders/vs.glsl"),
    GLT::Shader(GL_FRAGMENT_SHADER, "data/shaders/fs.glsl")});

//====[TEMPORARY]============================================================//

  // Create image texture
  Buffer2D<unsigned> iterationMap(width, height);

  // View positioning
  std::complex<float> center(-0.5, 0);
  std::complex<float> brotStart = center + std::complex<float>(-2, -1.5);
  std::complex<float> brotEnd = center + std::complex<float>(2, 1.5);

  // Maximum number of iterations
  unsigned maxIterations = 64;
  unsigned bailout = 2;
  unsigned superSampling = 4;

  // Compute iterations for all pixels
  SimpleBrot::ComputeIterations(
    iterationMap, brotStart, brotEnd, maxIterations, bailout, superSampling);

//====[TEMPORARY]============================================================//

  // Compute a mandelbrot image from iteration map
  Image image(width, height);
  for(int i = 0; i < height; i++) {
    for(int j = 0; j < width; j++) {
      float m = ((float)iterationMap.Get(j, i) / (float)maxIterations);
      unsigned char mag = m * 255;
      image.Get(j, i) = {mag, mag, mag};
    }
  }

  // Update internal opengl texture
  image.Update();

  std::string host = opt.Get("host");
  int port = opt.Get("port");

  // Draw loop
  while(!window.ShouldClose())
  {
    image.draw(shader);
    window.Refresh();
  }

  // El fin
  return 0;
}
