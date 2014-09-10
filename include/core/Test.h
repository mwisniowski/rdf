#ifndef RDF_TEST_H
#define RDF_TEST_H

#include "core/Interfaces.h"

/**
 * @brief Simple helper class to perform a binary test
 */
template< typename I, typename O, typename F >
class Test
{
  public:
    Test() :
      threshold_( NAN )
    {}

    Test( const F& feature,
        float threshold ) :
      threshold_( threshold ),
      feature_( feature )
    {}

    virtual ~Test()
    {}

    Test& operator=( const Test& other )
    {
      if( this != &other )
      {
        feature_ = other.feature_;
        threshold_ = other.threshold_;
      }
      return *this;
    }

    bool operator()( const DataPoint< I, O >& point ) const
    {
      return feature_( point.input() ) < threshold_;
    }

    friend std::ostream& operator<<( std::ostream& os, const Test& test )
    {
      os << test.feature_ << ", " << test.threshold_;
      return os;
    }

  private:
    F       feature_;
    float   threshold_;
};

#endif
