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
#define MPIBROT_UTIL_DISTRIBUTOR_RX_REQUEST_TAG 1
#define MPIBROT_UTIL_DISTRIBUTOR_TAG_COUNTER_BASE 10

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

    int const m_tx_request_tag = MPIBROT_UTIL_DISTRIBUTOR_TX_REQUEST_TAG;
    int const m_rx_request_tag = MPIBROT_UTIL_DISTRIBUTOR_RX_REQUEST_TAG;

    typedef struct
    {
      int rank;
      int ack_tag;
      int data_tag;
      bool stop;
    }
    TxRequestFrame;

    typedef struct
    {
      int rank;
      int ack_tag;
    }
    RxRequestFrame;

    typedef struct
    {
      int rank;
    }
    TxAckFrame;

    typedef struct
    {
      int rank;
      int data_tag;
      bool stop;
    }
    RxAckFrame;


  // Methods
  private:
    void sendTxRequest(TxRequestFrame const t_request, int const t_target)
    {
      mpi::error::check(MPI_Send(&t_request, sizeof(TxRequestFrame), MPI_BYTE, t_target, m_tx_request_tag, m_comm_all));
    }


    TxRequestFrame receiveTxRequest()
    {
      TxRequestFrame request;
      MPI_Status status;

      mpi::error::check(MPI_Recv(&request, sizeof(TxRequestFrame), MPI_BYTE, MPI_ANY_SOURCE, m_tx_request_tag, m_comm_all, &status));

      if(request.rank != status.MPI_SOURCE)
      {
        std::cout << "[Distributor] - Error, rank does not match source\n";
        exit(1);
      }

      return request;
    }


    void sendRxRequest(RxRequestFrame const t_request, int const t_target)
    {
      mpi::error::check(MPI_Send(&t_request, sizeof(RxRequestFrame), MPI_BYTE, t_target, m_rx_request_tag, m_comm_all));
    }


    RxRequestFrame receiveRxRequest()
    {
      RxRequestFrame request;
      MPI_Status status;

      mpi::error::check(MPI_Recv(&request, sizeof(RxRequestFrame), MPI_BYTE, MPI_ANY_SOURCE, m_rx_request_tag, m_comm_all, &status));

      if(request.rank != status.MPI_SOURCE)
      {
        std::cout << "[Distributor] - Error, rank does not match source\n";
        exit(1);
      }

      return request;
    }


    void sendTxAcknowledge(TxAckFrame const t_ack, int const t_target, int const t_tag)
    {
      mpi::error::check(MPI_Send(&t_ack, sizeof(TxAckFrame), MPI_BYTE, t_target, t_tag, m_comm_all));
    }


    TxAckFrame receiveTxAcknowledge(int const t_source, int const t_tag)
    {
      TxAckFrame ack;
      MPI_Status status;

      mpi::error::check(MPI_Recv(&ack, sizeof(TxAckFrame), MPI_BYTE, t_source, t_tag, m_comm_all, &status));

      return ack;
    }


    void sendRxAcknowledge(RxAckFrame const t_ack, int const t_target, int const t_tag)
    {
      mpi::error::check(MPI_Send(&t_ack, sizeof(RxAckFrame), MPI_BYTE, t_target, t_tag, m_comm_all));
    }


    RxAckFrame receiveRxAcknowledge(int const t_source, int const t_tag)
    {
      RxAckFrame ack;
      MPI_Status status;

      mpi::error::check(MPI_Recv(&ack, sizeof(RxAckFrame), MPI_BYTE, t_source, t_tag, m_comm_all, &status));

      return ack;
    }


    void signalHandlerMain()
    {
      TxRequestFrame tx_request;
      RxRequestFrame rx_request;

      while(1)
      {
        tx_request = this->receiveTxRequest();

        if(tx_request.stop == true)
        {
          break;
        }

        rx_request = this->receiveRxRequest();

        this->sendTxAcknowledge({rx_request.rank}, tx_request.rank, tx_request.ack_tag);
        this->sendRxAcknowledge({tx_request.rank, tx_request.data_tag}, rx_request.rank, rx_request.ack_tag);
      }
    }


    void receiveThreadMain(int const t_signal_handler_rank, int const t_ack_tag)
    {
      T rx_data;

      RxAckFrame rx_ack;
      RxRequestFrame const rx_request = {
        mpi::comm::rank(m_comm_all),
        t_ack_tag
      };

      while(1)
      {
        this->sendRxRequest(rx_request, t_signal_handler_rank);
        rx_ack = this->receiveRxAcknowledge(t_signal_handler_rank, t_ack_tag);

        if(rx_ack.stop == true)
        {
          break;
        }

        rx_data.mpiReceive(rx_ack.rank, rx_ack.data_tag, m_comm_all);

        m_output_queue->enqueue(rx_data);
      }
    }


    void transmitThreadMain(int const t_ack_tag, int const t_data_tag)
    {
      std::pair<int, T> tx_signal_data_pair;

      TxAckFrame tx_ack;
      TxRequestFrame const tx_request = {
        mpi::comm::rank(m_comm_all),
        t_ack_tag,
        t_data_tag,
        false
      };

      while(1)
      {
        tx_signal_data_pair = m_input_queue->dequeueWithSignal();

        if(tx_signal_data_pair.first == MPIBROT_UTIL_DISTRIBUTOR_STOP_SIGNAL)
        {
          break;
        }

        this->sendTxRequest(tx_request, m_my_signal_handler_rank);
        tx_ack = this->receiveTxAcknowledge(m_my_signal_handler_rank, t_ack_tag);

        tx_signal_data_pair.second.mpiSend(tx_ack.rank, t_data_tag, m_comm_all);
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
      mpi::error::check(MPI_Barrier(m_comm_all));

      int tag_counter = MPIBROT_UTIL_DISTRIBUTOR_TAG_COUNTER_BASE;

      // Start transmit threads
      for(unsigned i = 0; i < t_transmit_thread_count; i++)
      {
        int ack_tag = tag_counter++;
        int data_tag = tag_counter++;
        m_transmit_threads.push_back(std::thread(&util::Distributor<T>::transmitThreadMain, this, ack_tag, data_tag));
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
        int ack_tag = tag_counter++;
        m_receive_threads.push_back(std::thread(&util::Distributor<T>::receiveThreadMain, this, signal_handler_ranks.at(i), ack_tag));
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
      mpi::error::check(MPI_Barrier(m_comm_all));

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
      TxRequestFrame const signal_handler_stop_signal = {
        mpi::comm::rank(m_comm_all),
        0,
        0,
        true
      };

      for(unsigned i = 0; i < m_signal_handler_threads.size(); i++)
      {
        this->sendTxRequest(signal_handler_stop_signal, mpi::comm::rank(m_comm_all));
      }

      // Join signal handler threads
      for(unsigned i = 0; i < m_signal_handler_threads.size(); i++)
      {
        m_signal_handler_threads.at(i).join();
      }

      // Send stop signals to receive threads
      if(mpi::comm::rank(m_comm_all) == m_my_signal_handler_rank)
      {
        RxAckFrame const receieve_thread_stop_signal = {
          mpi::comm::rank(m_comm_all),
          0,
          true
        };

        for(int i = 0; i < mpi::comm::size(m_comm_all); i++)
        {
          RxRequestFrame request = this->receiveRxRequest();
          this->sendRxAcknowledge(receieve_thread_stop_signal, request.rank, request.ack_tag);
        }
      }

      // Join receive threads
      for(unsigned i = 0; i < m_receive_threads.size(); i++)
      {
        m_receive_threads.at(i).join();
      }

      // Destroy the internal communicator
      mpi::error::check(MPI_Comm_free(&m_comm_all));
    }
  };

} // namespace util


#endif // MPIBROT_UTIL_DISTRIBUTOR_INCLUDED
