
// Internal
#include "draw/Image.hpp"

// Standard
#include <stdlib.h>


// Initialise the things
Image::Image(unsigned const width, unsigned const height) :
  GLT::Texture2D(0, GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE, NULL),
  Buffer2D(width, height) {

  // Vertex positions, UVs and indices
  std::vector<GLT::vertex_t> const vertices = {
    {glm::fvec3(-1,-1, 0), glm::fvec2(0.0001, 0.0001)},
    {glm::fvec3( 1,-1, 0), glm::fvec2(1.0001, 0.0001)},
    {glm::fvec3(-1, 1, 0), glm::fvec2(0.0001, 1.0001)},
    {glm::fvec3( 1, 1, 0), glm::fvec2(1.0001, 1.0001)}};
  std::vector<unsigned> const indices = {
    0, 1, 2, 3, 2, 1};

  // Initialise vertex array
  this->vertexArray = GLT::VertexArray(vertices, indices);
}


// Resize the internal texture
void Image::Resize(unsigned const width, unsigned const height) {
  this->Bind();

  // Update data size
  Buffer2D<pixel_t>::Resize(width, height);

  // Update texture
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GL_RGB,
    this->width,
    this->height,
    0,
    GL_RGB,
    GL_UNSIGNED_BYTE,
    NULL);

  // Check for boo-boos
  if(glGetError() != GL_NO_ERROR) {
    std::cerr << "ERROR: Failed to resize iteration map texture\n";
    exit(1);
  }

  this->Unbind();
}


// Random orbit texture data
void Image::Random() {
  for(unsigned j = 0; j < this->height; j++) {
    for(unsigned i = 0; i < this->width; i++) {
      unsigned r = rand();
      this->Get(i, j).r = r % 255;
      this->Get(i, j).g = (r >> 8) % 255;
      this->Get(i, j).b = (r >> 16)  % 255;
    }
  }
}


// Update the internal texture
void Image::Update() {
  this->Bind();

  // Update texture
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GL_RGB,
    this->width,
    this->height,
    0,
    GL_RGB,
    GL_UNSIGNED_BYTE,
    this->data.data());

  // Check for boo-boos
  if(glGetError() != GL_NO_ERROR) {
    std::cerr << "ERROR: Failed to resize iteration map texture\n";
    exit(1);
  }

  this->Unbind();
}


// Draw method
void Image::Draw(GLT::ShaderProgram& shader) {
  shader.Use();
  glActiveTexture(GL_TEXTURE0);

  // Bind texture and set texture uniform
  this->Bind();
  shader.GetUniform("texture0").SetTex2D(0);

  // Draw the full screen quad mesh
  this->vertexArray.Bind();
  glDrawElements(
    GL_TRIANGLES,
    this->vertexArray.GetIndexBufferLength(),
    GL_UNSIGNED_INT, 0);
  this->vertexArray.Unbind();

  // Unbind orbit texture
  this->Unbind();
}
