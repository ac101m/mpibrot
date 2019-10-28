#ifndef MPIBROT_TXRX_INCLUDED
#define MPIBROT_TXRX_INCLUDED

// Standard
#include <vector>

// External
#include "boost/asio.hpp"


// Transmit arbitrary object
template<class T>
void Transmit(T const& data, boost::asio::ip::tcp::socket& socket) {
  write(socket, buffer(&data, sizeof(T)));
}


// Recieve arbirary object
template<class T>
T Recieve(boost::asio::ip::tcp::socket& socket) {
  T data;
  read(socket, buffer(&data, sizeof(T)));
  return data;
}


// Transmit arbitrary object vector
template<class T>
void Transmit(std::vector<T> const& data, boost::asio::ip::tcp::socket& socket) {
  Transmit<unsigned>(data.size(), socket);
  write(socket, buffer(data.data(), sizeof(T) * data.size()));
}


// Recieve arbitrary object vector
template<class T>
void Recieve(std::vector<T>* data, boost::asio::ip::tcp::socket& socket) {

  // Get number of elements to recieve first
  unsigned elementCount = Recieve<unsigned>(socket);

  // If neccessary, resize the vector
  if(data->size() != elementCount) {
    data->resize(elementCount);
  }

  // Read storage back
  read(socket, buffer(data->data(), sizeof(T)) * elementCount);
  return data;
}


#endif // MPIBROT_TXRX_INCLUDED