#ifndef MPIBROT_IMAGE_INCLUDED
#define MPIBROT_IMAGE_INCLUDED

// Standard
#include <vector>
#include <string>
#include <memory>

// External
#include "GLT/GL/ShaderProgram.hpp"
#include "GLT/GL/VertexArray.hpp"
#include "GLT/GL/Texture2D.hpp"


// Struct for pixel data
typedef struct {
  unsigned char r;
  unsigned char g;
  unsigned char b;
} pixel_t;


// Contains a texture representative
class Image : public GLT::Texture2D {
private:

  // Vertex data
  GLT::VertexArray vertexArray;

  // Size of image
  unsigned width, height;

  // Image data
  std::vector<pixel_t> data;
  std::vector<pixel_t*> indices;

//====[METHODS]==============================================================//

  // Initialise internal arrays
  void InitStorage();

public:

  Image();
  Image(unsigned const width, unsigned const height);

  void Resize(unsigned const width, unsigned const height);

  // Fill with random pixels
  void Random();

  // Update the internal texture object with new data
  void Update();

  // Draw method
  void Draw(GLT::ShaderProgram& shader);
};


#endif // MPIBROT_ORBITTEXTURE_INCLUDED
