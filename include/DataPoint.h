#ifndef CVT_RDF_DATAPOINT_H
#define CVT_RDF_DATAPOINT_H

#include <vector>
#include <istream>
#include <ostream>
#include <sstream>
#include <iostream>

using namespace std;

template< typename I, typename O, size_t d > 
struct DataPoint
{
  vector<I> input;
  O         output;


  DataPoint()
  {
    input.reserve( d );
  }

  DataPoint( vector<I>& in, O& out )
  {
    input = in;
    output = out;
  }

  virtual ~DataPoint()
  {
  }

  DataPoint& operator=( const DataPoint& other )
  {
    if( this != &other )
    {
      input = other.input;
      output = other.output;
    }
    return *this;
  }

  friend istream& operator>>( istream& is, DataPoint& point )
  {
    I input;

    is >> point.output;
    point.output -= 1;

    point.input.clear();
    for (int i = 0; i < d; ++i)
    {
      is >> input;
      point.input.push_back( input );
    }

    return is;
  }

  friend ostream& operator<<( ostream& os, const DataPoint& point )
  {
    os << "(";
    for (int i = 0; i < d - 1; ++i)
    {
      os << point.input[ i ] << ",";
    }
    os << point.input.back() << "),"; 

    os << point.output;

    return os;
  }
};

#endif
