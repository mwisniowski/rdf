#ifndef RDF_DATAPOINT_H

#define RDF_DATAPOINT_H

#include <vector>
#include <istream>
#include <ostream>
#include <sstream>
#include <iostream>

template< typename I, typename O > 
struct DataPoint
{
  public:
    DataPoint( const std::vector<I>& input, const O& output ) :
      input_( input ),
      output_( output )
    {}

    DataPoint( const DataPoint& other ) :
      input_( other.input_ ),
      output_( other.output_ )
    {}

    virtual ~DataPoint()
    {
    }

    DataPoint& operator=( const DataPoint& other )
    {
      if( this != &other )
      {
        input_ = other.input_;
        output_ = other.output_;
      }
      return *this;
    }

    friend std::ostream& operator<<( std::ostream& os, const DataPoint& point )
    {
      os << "(";
      for (int i = 0; i < point.input.size() - 1; i++ )
      {
        os << point.input_[ i ] << ",";
      }
      os << point.input_.back() << "),"; 

      os << point.output_;

      return os;
    }

    bool operator==( const DataPoint& other )
    {
      return input_ == other.input && output_ == other.output;
    }
    
    const I& input( size_t idx ) const
    {
      return input_[ idx ];
    }

    const std::vector< I >& input() const
    {
      return input_;
    }

    const O& output() const
    {
      return output_;
    }

  private:
    std::vector< I >  input_;
    O                 output_;
};

#endif
