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
  public:
    vector<I> input;
    O         output;
    size_t    id;

    DataPoint() :
      input( d, I( 0 ) ),
      output( O( 0 ) )
    {
    }

    DataPoint( const vector<I>& in, const O& out ) :
      input( in ),
      output( out )
    {
    }

    DataPoint( const DataPoint& other ) :
      input( other.input ),
      output( other.output ),
      id( other.id )
    {
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
        id = other.id;
      }
      return *this;
    }

    friend istream& operator>>( istream& is, DataPoint& point )
    {
      I input;

      is >> point.output;

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

    bool operator==( const DataPoint& other )
    {
      return input == other.input && output == other.output;
    }
};

#endif
