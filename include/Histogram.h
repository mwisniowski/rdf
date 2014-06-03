#ifndef AGGREGATOR_H
#define AGGREGATOR_H

#include <cvt/math/Math.h>
#include <map>
#include "DataCollection.h"

class Histogram
{
  public:
    size_t n;

  private:
    // map< u_int, float > statistics;
    vector< float > histogram;


  public:
    Histogram( size_t numClasses ) :
      n( 0 ),
      histogram( numClasses, 0.0f )
    {}

    Histogram( const Histogram& other ) :
      histogram( other.histogram ),
      n( other.n )
    {}

    virtual ~Histogram() 
    {}

    void aggregate( const DataRange& range )
    {
      DataCollection::const_iterator it( range.start );
      for( ; it != range.end; ++it )
      {
        histogram[ it->output ]++;
        n++;
      }
    }

    void aggregate( const Histogram& s )
    {
      //TODO ensure size
      vector< float >::iterator it = histogram.begin(), 
        end = histogram.end();
      vector< float >::const_iterator sit = s.histogram.begin();
      for( ; it != end; ++it, ++sit )
      {
        ( *it ) += ( *sit );
      }
      n += s.n;
    }

    pair< u_int, float > getMax() const
    {
      float maxValue = FLT_MIN;
      u_int maxC = 0;
      for( size_t i = 0; i < histogram.size(); i++ )
      {
        if( histogram[ i ] > maxValue )
        {
          maxValue = histogram[ i ];
          maxC = i;
        }
      }
      return pair< u_int, float >( maxC, maxValue / n );
    }

    float getEntropy() const
    {
      float entropy = 0.0f;

      for( size_t i = 0; i < histogram.size(); i++ )
      {
        if( float p_i = histogram[ i ] / n )
        {
          entropy += p_i * cvt::Math::log2( p_i );
        }
      }
      return -entropy;
    }

    float probability( size_t class_index ) const
    {
      return histogram[ class_index ] / n;
    }

    friend ostream& operator<<( ostream& os, const Histogram& s )
    {
      os << s.n << ": { ";
      for( size_t i = 0; i < s.histogram.size(); i++ )
      {
        os << "(" << i << "," << s.histogram[ i ] << ") ";
      }
      os << "}";

      return os;
    }


};

#endif
