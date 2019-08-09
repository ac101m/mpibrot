#ifndef MPIBROT_BUFFER2D_INCLUDED
#define MPIBROT_BUFFER2D_INCLUDED


// Standard
#include <vector>


template<class T>
class Buffer2D {
private:

  unsigned const width;
  unsigned const height;

  std::vector<T> data;
  std::vector<T*> indices;

public:

  // Initialise buffer
  Buffer2D(unsigned const width, unsigned const height) :
    data(std::vector<T>(width * height)),
    indices(std::vector<T*>(height)),
    width(width), height(height) {

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
