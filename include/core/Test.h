#ifndef RDF_TEST_H
#define RDF_TEST_H

#include "core/Interfaces.h"

/**
 * @brief Simple helper class to perform a binary test
 */
template< typename F, typename I >
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

    bool operator()( const I& input ) const
    {
      return feature_( input ) < threshold_;
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
