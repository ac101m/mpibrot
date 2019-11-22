#ifndef MPIBROT_UTIL_GATHERER_INCLUDED
#define MPIBROT_UTIL_GATHERER_INCLUDED


// Internal
#include "mpi/comm.hpp"
#include "util/Queue.hpp"

// External
#include "mpi.h"

// Standard
#include <thread>
#include <memory>
#include <iostream>


#define MPIBROT_UTIL_GATHERER_TX_REQUEST_TAG 0
#define MPIBROT_UTIL_GATHERER_TAG_COUNTER_BASE 10

#define MPIBROT_UTIL_GATHERER_STOP_SIGNAL -1


namespace util
{

  template<class T>
  class Gatherer
  {
  private:
    std::shared_ptr<util::Queue<T>> m_input_queue;
    std::shared_ptr<util::Queue<T>> m_output_queue;

    MPI_Comm m_comm;

    int const m_head_node;

    int const m_tx_request_tag;

    std::vector<std::thread> m_transmit_threads;
    std::vector<std::thread> m_receive_threads;

    typedef struct
    {
      int rank;
      int ack_tag;
      bool stop;
    }
    TxRequestFrame;

    typedef struct
    {
      int rank;
      int data_tag;
    }
    TxAckFrame;


  // Methods
  private:
    void sendTxRequest(TxRequestFrame const t_request)
    {
      mpi::error::check(MPI_Send(&t_request, sizeof(TxRequestFrame), MPI_BYTE, m_head_node, m_tx_request_tag, m_comm));
    }


    TxRequestFrame receiveTxRequest()
    {
      TxRequestFrame request;
      MPI_Status status;

      mpi::error::check(MPI_Recv(&request, sizeof(TxRequestFrame), MPI_BYTE, MPI_ANY_SOURCE, m_tx_request_tag, m_comm, &status));

      if(request.rank != status.MPI_SOURCE)
      {
        std::cout << "[Scatterer] - Error, rank does not match source\n";
        exit(1);
      }

      return request;
    }


    void sendTxAcknowledge(TxAckFrame const t_ack, int const t_destination, int const t_tag)
    {
      mpi::error::check(MPI_Send(&t_ack, sizeof(TxAckFrame), MPI_BYTE, t_destination, t_tag, m_comm));
    }


    TxAckFrame receiveTxAcknowledge(int const t_source, int const t_tag)
    {
      TxAckFrame ack;
      MPI_Status status;

      mpi::error::check(MPI_Recv(&ack, sizeof(TxAckFrame), MPI_BYTE, t_source, t_tag, m_comm, &status));

      if(ack.rank != status.MPI_SOURCE)
      {
        std::cout << "[Scatterer] - Error, rank does not match source\n";
        exit(1);
      }

      return ack;
    }

    void transmitThreadMain(int const t_ack_tag)
    {
      std::pair<int, T> signal_data_pair;
      TxAckFrame ack;

      TxRequestFrame const tx_request = {
        mpi::comm::rank(m_comm),
        t_ack_tag,
        false
      };

      while(1)
      {
        signal_data_pair = m_input_queue->dequeueWithSignal();

        if(signal_data_pair.first == MPIBROT_UTIL_GATHERER_STOP_SIGNAL)
        {
          break;
        }

        this->sendTxRequest(tx_request);
        ack = this->receiveTxAcknowledge(m_head_node, t_ack_tag);

        if(ack.rank != m_head_node)
        {
          std::cout << "[Gatherer] - Error, invalid rx rank\n";
          exit(1);
        }

        signal_data_pair.second.mpiSend(m_head_node, ack.data_tag, m_comm);
      }
    }


    void receiveThreadMain(int const t_data_tag)
    {
      TxRequestFrame request;
      T tx_data;

      while(1)
      {
        request = this->receiveTxRequest();

        if(request.stop == true)
        {
          break;
        }

        this->sendTxAcknowledge({mpi::comm::rank(m_comm), t_data_tag}, request.rank, request.ack_tag);

        tx_data.mpiReceive(request.rank, t_data_tag, m_comm);
        m_output_queue->enqueue(tx_data);
      }
    }


  // Methods
  public:
    Gatherer(
      std::shared_ptr<util::Queue<T>> t_input_queue,
      std::shared_ptr<util::Queue<T>> t_output_queue,
      MPI_Comm const t_communicator,
      int const t_head_node = 0,
      unsigned const t_transmit_thread_count = 1,
      unsigned const t_receive_thread_count = 1) :
      m_input_queue(t_input_queue),
      m_output_queue(t_output_queue),
      m_comm(mpi::comm::duplicate(t_communicator)),
      m_head_node(t_head_node),
      m_tx_request_tag(MPIBROT_UTIL_GATHERER_TX_REQUEST_TAG)
    {
      int tag_counter = MPIBROT_UTIL_GATHERER_TAG_COUNTER_BASE;

      if(mpi::comm::rank(m_comm) != m_head_node)
      {
        if(m_output_queue != nullptr)
        {
          std::cout << "[Gatherer] Warning, output queue passed to gatherer on rank ";
          std::cout << mpi::comm::rank(m_comm) << " but head node is " << m_head_node;
          std::cout << ". This queue will never be enqueued, this is probably not what you wanted!\n";
        }
      }

      // Tranmit on all nodes
      for(unsigned i = 0; i < t_transmit_thread_count; i++)
      {
        int tag = tag_counter++;
        m_transmit_threads.push_back(std::thread(&util::Gatherer<T>::transmitThreadMain, this, tag));
      }

      // Receieve on head node
      if(mpi::comm::rank(m_comm) == m_head_node)
      {
        for(unsigned i = 0; i < t_receive_thread_count; i++)
        {
          int tag = tag_counter++;
          m_receive_threads.push_back(std::thread(&util::Gatherer<T>::receiveThreadMain, this, tag));
        }
      }
    }


    // Moveable but not copyable
    Gatherer(Gatherer const &) = delete;
    Gatherer& operator=(Gatherer const &) = delete;


    ~Gatherer()
    {
      if(mpi::comm::rank(m_comm) == m_head_node)
      {
        TxRequestFrame const stop_signal = {
          mpi::comm::rank(m_comm),
          0,
          true
        };

        for(unsigned i = 0; i < m_receive_threads.size(); i++)
        {
          this->sendTxRequest(stop_signal);
        }

        for(unsigned i = 0; i < m_receive_threads.size(); i++)
        {
          m_receive_threads[i].join();
        }
      }

      for(unsigned i = 0; i < m_transmit_threads.size(); i++)
      {
        m_input_queue->enqueueWithSignal(std::make_pair(MPIBROT_UTIL_GATHERER_STOP_SIGNAL, T()));
      }

      for(unsigned i = 0; i < m_transmit_threads.size(); i++)
      {
        m_transmit_threads[i].join();
      }

      mpi::error::check(MPI_Comm_free(&m_comm));
    }
  };

} // namespace util


#endif // MPIBROT_UTIL_GATHERER_INCLUDED
