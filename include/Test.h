#ifndef CVT_RDF_TEST_H
#define CVT_RDF_TEST_H 

#include "DataPoint.h"

using namespace std;

template< typename I, size_t in_size >
struct Test
{
public:
  Test( vector< I > feature_parameters, I threshold ) :
    _phi( feature_parameters ),
    _tau( threshold )
  {}

  virtual ~Test() 
  {}

  bool operator()( vector< I >& feature_vector )
  {
    I sum( 0 );
    for( size_t i = 0; i < in_size; i++ )
    {
      sum += feature_vector[ i ] * _phi[ i ];
    }

    return sum > _tau;
  }

private:
  vector< I > _phi;
  I           _tau;
};

#endif
