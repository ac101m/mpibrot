#ifndef MPIBROT_UTIL_MPI_ERROR_INCLUDED
#define MPIBROT_UTIL_MPI_ERROR_INCLUDED


// External
#include "mpi.h"

// Standard
#include <string>
#include <exception>


namespace mpi
{
  namespace error
  {

    inline char * c_string(int const t_errorcode)
    {
      char * error_string = new char[MPI_MAX_ERROR_STRING];
      int error_string_length;

      MPI_Error_string(t_errorcode, error_string, &error_string_length);

      return error_string;
    }


    inline std::string string(int const t_errorcode)
    {
      return std::string(c_string(t_errorcode));
    }


    // A throwable object to use in conjunction with mpi errors
    class MpiException : public std::exception
    {
    private:
      int const m_errorcode;

    public:
      MpiException(int const t_errorcode) :
        m_errorcode(t_errorcode)
      {}

      const char * what() const throw()
      {
        return c_string(m_errorcode);
      }
    };


    inline void check(int const t_errorcode)
    {
      if(t_errorcode != MPI_SUCCESS)
      {
        throw MpiException(t_errorcode);
      }
    }

  } // namespace error

} // namespace mpi


#endif // MPIBROT_UTIL_MPI_ERROR_INCLUDED
