#ifndef MPIBROT_SIMPLEBROT_INCLUDED
#define MPIBROT_SIMPLEBROT_INCLUDED

// Internal
#include "util/Buffer2D.hpp"

// Standard
#include <complex>


namespace SimpleBrot {

  // Compute a single pixel orbit
  template<class T>
  unsigned ComputeIterationCount(
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
  void FillIterationBuffer(
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

        data.Get(j, i) = ComputeIterationCount(c, maxIterations, bailout);
      }
    }
  }


  // Supersampled iteration count
  template<class T>
  void ComputeIterations(
    Buffer2D<unsigned>& data,
    std::complex<T> const start,
    std::complex<T> const end,
    unsigned const maxIterations,
    unsigned const bailout,
    unsigned const scale) {

    // Generate a scaled buffer and compute iterations into it
    Buffer2D<unsigned> superSampleBuffer(data.Width() * scale, data.Height() * scale);
    FillIterationBuffer(
      superSampleBuffer, start, end, maxIterations, bailout);

    // Iterate over output buffer computing sample averages
    for(unsigned i = 0; i < data.Height(); i++) {
      for(unsigned j = 0; j < data.Width(); j++) {
        float sum = 0;
        for(unsigned k = 0; k < scale; k++) {
          for(unsigned l = 0; l < scale; l++) {
            sum += superSampleBuffer.Get(j * scale + l, i * scale + k);
          }
        }
        data.Get(j, i) = sum / pow(scale, 2);
      }
    }
  }

} // namespace SimpleBrot


#endif // MPIBROT_SIMPLEBROT_INCLUDED
