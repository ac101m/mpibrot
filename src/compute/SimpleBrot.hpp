#ifndef MPIBROT_SIMPLEBROT_INCLUDED
#define MPIBROT_SIMPLEBROT_INCLUDED

// Internal
#include "util/Buffer2D.hpp"

// Standard
#include <complex>


namespace SimpleBrot {

  // Compute a single pixel orbit
  template<class T>
  unsigned ComputeOrbit(
    std::complex<T> const& c,
    unsigned const maxIterations,
    unsigned const bailout) {

    std::complex<T> z;
    unsigned iterationCount = 0;

    while(abs(z) < bailout && iterationCount < maxIterations) {
      z = pow(z, 2) + c;
      iterationCount++;
    }

    return iterationCount;
  }


  // Generate a 2d buffer full of iteration counts
  template<class T>
  void ComputeIterations(
    Buffer2D<unsigned>& data,
    std::complex<T> const start,
    std::complex<T> const end,
    unsigned const maxIterations,
    unsigned const bailout) {

    // Compute step sizes for pixels
    float rStep = (end.real() - start.real()) / data.Width();
    float iStep = (end.imag() - start.imag()) / data.Height();

    // Iterate over pixels in buffer
    for(unsigned i = 0; i < data.Height(); i++) {
      for(unsigned j = 0; j < data.Width(); j++) {

        std::complex<T> c = std::complex<T>(
          start.real() + (rStep * j),
          start.imag() + (iStep * i));

        data.Get(j, i) = ComputeOrbit(c, maxIterations, bailout);
      }
    }
  }

} // namespace SimpleBrot


#endif // MPIBROT_SIMPLEBROT_INCLUDED
