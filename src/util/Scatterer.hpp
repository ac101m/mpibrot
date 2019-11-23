#ifndef MPIBROT_UTIL_SCATTERER_INCLUDED
#define MPIBROT_UTIL_SCATTERER_INCLUDED


// Internal
#include "mpi/comm.hpp"
#include "util/Queue.hpp"

// External
#include "mpi.h"

// Standard
#include <thread>
#include <memory>
#include <iostream>


#define MPIBROT_UTIL_SCATTERER_RX_REQUEST_TAG 0
#define MPIBROT_UTIL_SCATTERER_TAG_COUNTER_BASE 10

#define MPIBROT_UTIL_SCATTERER_STOP_SIGNAL -1


namespace util
{

  template<class T>
  class Scatterer
  {
  private:
    std::shared_ptr<util::Queue<T>> m_input_queue;
    std::shared_ptr<util::Queue<T>> m_output_queue;

    MPI_Comm m_comm;

    int const m_head_node;

    int const m_rx_request_tag;

    std::vector<std::thread> m_transmit_threads;
    std::vector<std::thread> m_receive_threads;

    typedef struct
    {
      int rank;
      int ack_tag;
      int data_tag;
    }
    RxRequestFrame;

    typedef struct
    {
      int rank;
      bool stop;
    }
    RxAckFrame;


  // Methods
  private:
    void sendRxRequest(RxRequestFrame const t_request)
    {
      mpi::error::check(MPI_Send(&t_request, sizeof(RxRequestFrame), MPI_BYTE, m_head_node, m_rx_request_tag, m_comm));
    }


    RxRequestFrame receiveRxRequest()
    {
      RxRequestFrame request;
      MPI_Status status;

      mpi::error::check(MPI_Recv(&request, sizeof(RxRequestFrame), MPI_BYTE, MPI_ANY_SOURCE, m_rx_request_tag, m_comm, &status));

      if(request.rank != status.MPI_SOURCE)
      {
        std::cout << "[Scatterer] - Error, request rank does not match source\n";
        exit(1);
      }

      return request;
    }


    void sendRxAcknowledge(RxAckFrame const t_ack, int const t_target, int const t_tag)
    {
      mpi::error::check(MPI_Send(&t_ack, sizeof(RxAckFrame), MPI_BYTE, t_target, t_tag, m_comm));
    }


    RxAckFrame receiveRxAcknowledge(int const t_tag)
    {
      MPI_Status status;
      RxAckFrame ack;

      mpi::error::check(MPI_Recv(&ack, sizeof(RxAckFrame), MPI_BYTE, m_head_node, t_tag, m_comm, &status));

      if(ack.rank != status.MPI_SOURCE)
      {
        std::cout << "[Scatterer] - Error, acknowledge rank does not match source\n";
        exit(1);
      }

      return ack;
    }


    void transmitThreadMain()
    {
      std::pair<int, T> signal_data_pair;
      RxRequestFrame rx_request;

      RxAckFrame const rx_ack = {
        mpi::comm::rank(m_comm),
        false
      };

      while(1)
      {
        signal_data_pair = m_input_queue->dequeueWithSignal();

        if(signal_data_pair.first == MPIBROT_UTIL_SCATTERER_STOP_SIGNAL)
        {
          break;
        }

        rx_request = this->receiveRxRequest();
        this->sendRxAcknowledge(rx_ack, rx_request.rank, rx_request.ack_tag);

        signal_data_pair.second.mpiSend(rx_request.rank, rx_request.data_tag, m_comm);
      }
    }


    void receiveThreadMain(int const t_ack_tag, int const t_data_tag)
    {
      T rx_data;
      RxAckFrame rx_ack;

      RxRequestFrame const rx_request = {
        mpi::comm::rank(m_comm),
        t_ack_tag,
        t_data_tag
      };

      while(1)
      {
        this->sendRxRequest(rx_request);
        rx_ack = this->receiveRxAcknowledge(t_ack_tag);

        if(rx_ack.stop == true)
        {
          break;
        }

        if(rx_ack.rank != m_head_node)
        {
          std::cout << "[Scatterer] Error, received invalid tx rank\n";
          exit(1);
        }

        rx_data.mpiReceive(rx_ack.rank, t_data_tag, m_comm);
        m_output_queue->enqueue(rx_data);
      }
    }


  // Methods
  public:
    Scatterer(
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
      m_rx_request_tag(MPIBROT_UTIL_SCATTERER_RX_REQUEST_TAG)
    {
      // Constructor muct be called collectively
      mpi::error::check(MPI_Barrier(m_comm));

      int tag_counter = MPIBROT_UTIL_SCATTERER_TAG_COUNTER_BASE;

      if(mpi::comm::rank(m_comm) != m_head_node)
      {
        if(t_input_queue != nullptr)
        {
          std::cout << "[Scatterer] Warning, input queue passed to scatterer on rank ";
          std::cout << mpi::comm::rank(m_comm) << " but head node is " << m_head_node;
          std::cout << ". Any input on this queue will be ignored, this is probably not what you wanted!\n";
        }
      }

      // Transmit on head node
      if(mpi::comm::rank(m_comm) == m_head_node)
      {
        for(unsigned i = 0; i < t_transmit_thread_count; i++)
        {
          m_transmit_threads.push_back(std::thread(&util::Scatterer<T>::transmitThreadMain, this));
        }
      }

      // Recieve on all nodes
      for(unsigned i = 0; i < t_receive_thread_count; i++)
      {
        int ack_tag = tag_counter++;
        int data_tag = tag_counter++;
        m_receive_threads.push_back(std::thread(&util::Scatterer<T>::receiveThreadMain, this, ack_tag, data_tag));
      }
    }


    // Moveable but not copyable
    Scatterer(Scatterer const &) = delete;
    Scatterer& operator=(Scatterer const &) = delete;


    ~Scatterer()
    {
      // Destructor must be called collectively
      mpi::error::check(MPI_Barrier(m_comm));

      if(mpi::comm::rank(m_comm) == m_head_node)
      {
        for(unsigned i = 0; i < m_transmit_threads.size(); i++)
        {
          m_input_queue->enqueueWithSignal(std::make_pair(MPIBROT_UTIL_SCATTERER_STOP_SIGNAL, T()));
        }

        for(unsigned i = 0; i < m_transmit_threads.size(); i++)
        {
          m_transmit_threads[i].join();
        }

        RxAckFrame const rx_stop_signal = {
          mpi::comm::rank(m_comm),
          true
        };

        for(unsigned i = 0; i < m_receive_threads.size() * mpi::comm::size(m_comm); i++)
        {
          RxRequestFrame rx_request = this->receiveRxRequest();
          this->sendRxAcknowledge(rx_stop_signal, rx_request.rank, rx_request.ack_tag);
        }
      }

      for(unsigned i = 0; i < m_receive_threads.size(); i++)
      {
        m_receive_threads[i].join();
      }

      mpi::error::check(MPI_Comm_free(&m_comm));
    }
  };

} // namespace util


#endif // MPIBROT_UTIL_SCATTERER_INCLUDED
