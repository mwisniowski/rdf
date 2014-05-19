#ifndef CVT_RDF_DATAPOINT_H
#define CVT_RDF_DATAPOINT_H

#include <vector>
#include <istream>
#include <ostream>
#include <sstream>
#include <iostream>

using namespace std;

template< typename I, typename O, size_t in_size > 
class DataPoint
{
public:
  DataPoint()
  {
    _input.reserve( in_size );
  }

  virtual ~DataPoint()
  {
  }

  inline vector<I>& input()
  {
    return _input;
  }

  inline O& output()
  {
    return _output;
  }

  friend istream& operator>>( istream& is, DataPoint& point )
  {
    string line;
    getline( is, line );
    istringstream iss( line );
    I input;

    iss >> point._output;

    for (int i = 0; i < in_size; ++i)
    {
      iss >> input;
      point._input.push_back( input );
    }

    return is;
  }

  friend ostream& operator<<( ostream& os, const DataPoint& instance )
  {
    os << "(";
    for (int i = 0; i < in_size - 1; ++i)
    {
      os << instance._input[ i ] << ",";
    }
    os << instance._input.back() << "),"; 

    os << instance._output;

    return os;
  }

private:
  vector<I> _input;
  O         _output;
};

typedef DataPoint< float, u_int, 2 > DataPoint2f;

#endif
