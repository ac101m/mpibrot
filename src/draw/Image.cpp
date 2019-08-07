#include "draw/Image.hpp"

// Standard
#include <stdlib.h>


// Initialise the things
Image::Image(unsigned const width, unsigned const height) :
  GLT::Texture2D(0, GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE, NULL),
  width(width), height(height) {

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

  // Allocate data memory
  this->data = std::vector<pixel_t>(width * height);
}


// Resize the internal texture
void Image::Resize(unsigned const width, unsigned const height) {
  this->Bind();

  // Update size
  this->width = width;
  this->height = height;

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

  // Regenerate data array
  this->data = std::vector<pixel_t>(width * height);
}


// Random orbit texture data
void Image::Random() {
  for(unsigned i = 0; i < this->data.size(); i++) {
    this->data[i].r = rand() % 255;
    this->data[i].g = rand() % 255;
    this->data[i].b = rand() % 255;
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
