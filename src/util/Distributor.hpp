#ifndef MPIBROT_UTIL_DISTRIBUTOR_INCLUDED
#define MPIBROT_UTIL_DISTRIBUTOR_INCLUDED


// Internal
#include "util/Queue.hpp"
#include "mpi/error.hpp"
#include "mpi/comm.hpp"

// External
#include "mpi.h"

// Standard
#include <vector>
#include <memory>
#include <thread>
#include <iostream>


// Communication tags
#define MPIBROT_UTIL_DISTRIBUTOR_TX_REQUEST_TAG 0
#define MPIBROT_UTIL_DISTRIBUTOR_TX_RESPONSE_TAG 1
#define MPIBROT_UTIL_DISTRIBUTOR_RX_REQUEST_TAG 2
#define MPIBROT_UTIL_DISTRIBUTOR_RX_RESPONSE_TAG 3
#define MPIBROT_UTIL_DISTRIBUTOR_DATA_TAG 4

// Stop signal
#define MPIBROT_UTIL_DISTRIBUTOR_STOP_SIGNAL -1


namespace util
{

  template<class T>
  class Distributor
  {
  private:
    std::shared_ptr<util::Queue<T>> m_input_queue;
    std::shared_ptr<util::Queue<T>> m_output_queue;

    MPI_Comm m_comm_all;

    std::vector<std::thread> m_signal_handler_threads;
    std::vector<std::thread> m_transmit_threads;
    std::vector<std::thread> m_receive_threads;

    int const m_signal_group_size;
    int const m_my_signal_group;
    int const m_my_signal_handler_rank;


  // Methods
  private:
    void sendInt(int const t_destination, int const t_tag, int const t_value)
    {
      mpi::error::check(MPI_Send(&t_value, 1, MPI_INT, t_destination, t_tag, m_comm_all));
    }


    int receiveInt(int const t_source, int const t_tag)
    {
      int value;
      MPI_Status status;
      mpi::error::check(MPI_Recv(&value, 1, MPI_INT, t_source, t_tag, m_comm_all, &status));
      return value;
    }


    void sendRankNumber(int const t_destination, int const t_tag)
    {
      this->sendInt(t_destination, t_tag, mpi::comm::rank(m_comm_all));
    }


    int receiveRankNumber(int const t_source, int const t_tag)
    {
      return this->receiveInt(t_source, t_tag);
    }


    void sendRxRequest(int const t_signal_handler_rank)
    {
      this->sendRankNumber(t_signal_handler_rank, MPIBROT_UTIL_DISTRIBUTOR_RX_REQUEST_TAG);
    }


    int receiveRxRequest()
    {
      return this->receiveRankNumber(MPI_ANY_SOURCE, MPIBROT_UTIL_DISTRIBUTOR_RX_REQUEST_TAG);
    }


    void sendRxResponse(int const t_destination, int const t_value)
    {
      this->sendInt(t_destination, MPIBROT_UTIL_DISTRIBUTOR_RX_RESPONSE_TAG, t_value);
    }


    int receiveRxResponse(int const t_source)
    {
      return this->receiveInt(t_source, MPIBROT_UTIL_DISTRIBUTOR_RX_RESPONSE_TAG);
    }


    void sendTxRequest(int const t_destination)
    {
      this->sendRankNumber(t_destination, MPIBROT_UTIL_DISTRIBUTOR_TX_REQUEST_TAG);
    }


    void sendTxStopRequest(int const t_destination)
    {
      this->sendInt(t_destination, MPIBROT_UTIL_DISTRIBUTOR_TX_REQUEST_TAG, MPIBROT_UTIL_DISTRIBUTOR_STOP_SIGNAL);
    }


    int receiveTxRequest()
    {
      return this->receiveRankNumber(MPI_ANY_SOURCE, MPIBROT_UTIL_DISTRIBUTOR_TX_REQUEST_TAG);
    }


    void sendTxResponse(int const t_destination, int const t_response)
    {
      this->sendInt(t_destination, MPIBROT_UTIL_DISTRIBUTOR_TX_RESPONSE_TAG, t_response);
    }


    int receiveTxResponse(int const t_source)
    {
      return this->receiveInt(t_source, MPIBROT_UTIL_DISTRIBUTOR_TX_RESPONSE_TAG);
    }


    void signalHandlerMain()
    {
      int tx_request_rank, rx_request_rank;

      while(1)
      {
        tx_request_rank = this->receiveTxRequest();

        if(tx_request_rank == MPIBROT_UTIL_DISTRIBUTOR_STOP_SIGNAL)
        {
          break;
        }

        rx_request_rank = this->receiveRxRequest();

        this->sendTxResponse(tx_request_rank, rx_request_rank);
        this->sendRxResponse(rx_request_rank, tx_request_rank);
      }
    }


    void receiveThreadMain(int const t_signal_handler_rank)
    {
      T rx_data;
      int tx_rank;

      while(1)
      {
        this->sendRxRequest(t_signal_handler_rank);
        tx_rank = this->receiveRxResponse(t_signal_handler_rank);

        if(tx_rank == MPIBROT_UTIL_DISTRIBUTOR_STOP_SIGNAL)
        {
          break;
        }

        rx_data.mpiReceive(tx_rank, MPIBROT_UTIL_DISTRIBUTOR_DATA_TAG, m_comm_all);

        m_output_queue->enqueue(rx_data);
      }
    }


    void transmitThreadMain()
    {
      std::pair<int, T> tx_signal_data_pair;
      int rx_rank;

      while(1)
      {
        tx_signal_data_pair = m_input_queue->dequeueWithSignal();

        if(tx_signal_data_pair.first == MPIBROT_UTIL_DISTRIBUTOR_STOP_SIGNAL)
        {
          break;
        }

        this->sendTxRequest(m_my_signal_handler_rank);
        rx_rank = this->receiveTxResponse(m_my_signal_handler_rank);
        tx_signal_data_pair.second.mpiSend(rx_rank, MPIBROT_UTIL_DISTRIBUTOR_DATA_TAG, m_comm_all);
      }
    }


  // Methods
  public:
    Distributor(
      std::shared_ptr<util::Queue<T>> t_input_queue,
      std::shared_ptr<util::Queue<T>> t_output_queue,
      MPI_Comm const t_basis_communicator,
      unsigned const t_signal_group_count = 1,
      unsigned const t_transmit_thread_count = 1,
      unsigned const t_signal_thread_count = 1) :
      m_input_queue(t_input_queue),
      m_output_queue(t_output_queue),
      m_comm_all(mpi::comm::duplicate(t_basis_communicator)),
      m_signal_group_size((mpi::comm::size(m_comm_all) + (t_signal_group_count / 2)) / t_signal_group_count),
      m_my_signal_group(mpi::comm::rank(m_comm_all) / m_signal_group_size),
      m_my_signal_handler_rank(m_my_signal_group * m_signal_group_size)
    {
      // Start transmit threads
      for(unsigned i = 0; i < t_transmit_thread_count; i++)
      {
        m_transmit_threads.push_back(std::thread(&util::Distributor<T>::transmitThreadMain, this));
      }

      // Get vector of all ranks with signal handlers running on them
      std::vector<int> signal_handler_ranks;
      for(int i = 0; i < mpi::comm::size(m_comm_all); i++)
      {
        if((mpi::comm::rank(m_comm_all) % m_signal_group_size) == 0)
        {
          signal_handler_ranks.push_back(mpi::comm::rank(m_comm_all));
        }
      }

      // Start one receive thread for each signal handler rank
      // pass it the rank of the signal handler it should listen to
      for(unsigned i = 0; i < signal_handler_ranks.size(); i++)
      {
        m_receive_threads.push_back(std::thread(&util::Distributor<T>::receiveThreadMain, this, signal_handler_ranks.at(i)));
      }

      // Start signal handlers
      if(mpi::comm::rank(m_comm_all) == m_my_signal_handler_rank)
      {
        for(unsigned i = 0; i < t_signal_thread_count; i++)
        {
          m_signal_handler_threads.push_back(std::thread(&util::Distributor<T>::signalHandlerMain, this));
        }
      }
    }


    // Moveable but not copyable
    Distributor(Distributor const &) = delete;
    Distributor& operator=(Distributor const &) = delete;


    ~Distributor()
    {
      // Send stop signals to transmit threads
      for(unsigned i = 0; i < m_transmit_threads.size(); i++)
      {
        this->m_input_queue->enqueueWithSignal(std::make_pair(MPIBROT_UTIL_DISTRIBUTOR_STOP_SIGNAL, T()));
      }

      // Join transmit threads
      for(unsigned i = 0; i < m_transmit_threads.size(); i++)
      {
        m_transmit_threads.at(i).join();
      }

      // Send stop signals to signal handlers on this rank
      for(unsigned i = 0; i < m_signal_handler_threads.size(); i++)
      {
        this->sendTxStopRequest(mpi::comm::rank(m_comm_all));
      }

      // Join signal handler threads
      for(unsigned i = 0; i < m_signal_handler_threads.size(); i++)
      {
        m_signal_handler_threads.at(i).join();
      }

      // Send stop signals to receive threads
      if(mpi::comm::rank(m_comm_all) == m_my_signal_handler_rank)
      {
        for(int i = 0; i < mpi::comm::size(m_comm_all); i++)
        {
          int rx_rank = this->receiveRxRequest();
          this->sendRxResponse(rx_rank, MPIBROT_UTIL_DISTRIBUTOR_STOP_SIGNAL);
        }
      }

      // Join receive threads
      for(unsigned i = 0; i < m_receive_threads.size(); i++)
      {
        m_receive_threads.at(i).join();
      }

      mpi::error::check(MPI_Comm_free(&m_comm_all));
    }
  };

} // namespace util


#endif // MPIBROT_UTIL_DISTRIBUTOR_INCLUDED
