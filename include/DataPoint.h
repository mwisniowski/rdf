#ifndef CVT_RDF_DATAPOINT_H
#define CVT_RDF_DATAPOINT_H

#include <vector>
#include <istream>
#include <ostream>
#include <sstream>
#include <iostream>

template< typename I, typename O > 
struct DataPoint
{
  public:
    std::vector<I> input;
    O         output;

    DataPoint()
    {}

    DataPoint( const std::vector<I>& in, const O& out ) :
      input( in ),
      output( out )
    {}

    DataPoint( const DataPoint& other ) :
      input( other.input ),
      output( other.output )
    {}

    virtual ~DataPoint()
    {
    }

    DataPoint& operator=( const DataPoint& other )
    {
      if( this != &other )
      {
        input = other.input;
        output = other.output;
        // id = other.id;
      }
      return *this;
    }

    friend std::ostream& operator<<( std::ostream& os, const DataPoint& point )
    {
      os << "(";
      for (int i = 0; i < point.input.size() - 1; i++ )
      {
        os << point.input[ i ] << ",";
      }
      os << point.input.back() << "),"; 

      os << point.output;

      return os;
    }

    bool operator==( const DataPoint& other )
    {
      return input == other.input && output == other.output;
    }
};

#endif
