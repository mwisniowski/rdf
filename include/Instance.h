#include <vector>
#include <istream>
#include <ostream>
#include <sstream>

template< typename I, typename O, size_t in_size, size_t out_size > 
class Instance
{
public:
  Instance()
  {
    _input.reserve( in_size );
    _output.reserve( out_size );
  }

  virtual ~Instance()
  {
  }

  inline std::vector<I>& input()
  {
    return _input;
  }

  inline std::vector<O>& output()
  {
    return _output;
  }

  friend std::istream& operator>>( std::istream& is, Instance& instance )
  {
    std::string line, ignore;
    std::getline( is, line );
    std::istringstream iss( line );

    for (int i = 0; i < in_size; ++i)
    {
      iss >> instance._input[ i ];
    }

    for (int i = 0; i < out_size; ++i)
    {
      iss >> instance._output[ i ];
    }

    return is;
  }

  friend std::ostream& operator<<( std::ostream& os, const Instance& instance )
  {
    os << "(";
    for (int i = 0; i < instance._input.size() - 1; ++i)
    {
      os << instance._input[ i ] << ",";
    }
    os << instance._input.back() << "),("; 

    for (int i = 0; i < instance._output.size() - 1; ++i)
    {
      os << instance._output[ i ] << ",";
    }
    os << instance._output.back() << ")"; 

    return os;
  }

private:
  std::vector<I> _input;
  std::vector<O> _output;
};

