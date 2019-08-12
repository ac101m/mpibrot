#ifndef MPIBROT_BUFFER2D_INCLUDED
#define MPIBROT_BUFFER2D_INCLUDED


// Standard
#include <vector>


template<class T>
class Buffer2D {
protected:

  unsigned width;
  unsigned height;

  std::vector<T> data;
  std::vector<T*> indices;

public:

  // Don't initialise buffer
  Buffer2D() : width(0), height(0) {}

  // Initialise buffer
  Buffer2D(unsigned const width, unsigned const height) :
    width(width), height(height),
    data(std::vector<T>(width * height)),
    indices(std::vector<T*>(height)) {

    // Initialise row indices
    for(unsigned i = 0; i < this->indices.size(); i++) {
      this->indices[i] = &this->data[i * width];
    }
  }

  // Resize the buffer, destroys data
  void Resize(unsigned const width, unsigned const height) {
    this->width = width;
    this->height = height;
    this->data = std::vector<T>(width * height);
    this->indices = std::vector<T*>(height);

    // Initialise row indices
    for(unsigned i = 0; i < this->indices.size(); i++) {
      this->indices[i] = &this->data[i * width];
    }
  }

  // Size gets
  unsigned Width() {return this->width;}
  unsigned Height() {return this->height;}

  // Get reference to arbitrary element
  T& Get(unsigned const x, unsigned const y) {
    return this->indices.at(y)[x];
  }
};


#endif // MPIBROT_BUFFER2D_INCLUDED
