#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <cvt/math/Math.h>
#include <map>

#include "DataRange.h"
#include "Interfaces.h"

template< typename D >
class Histogram: public IStatistics< D, Histogram< D > >
{
  public:
    size_t n;

  public:
    typedef std::vector< size_t >      histogram_type;
    histogram_type                     histogram;


  public:
    Histogram( size_t numClasses ) :
      histogram( numClasses ),
      n( 0 )
    {}

    Histogram( const Histogram& other ) :
      histogram( other.histogram ),
      n( other.n )
    {}

    virtual ~Histogram() 
    {}

    Histogram& operator=( const Histogram& other )
    {
      if( this != &other )
      {
        n = other.n;
        histogram = other.histogram;
      }
      return *this;
    }

    /**
     * @brief Adds DataPoints from range to histogram
     *
     * @param range
     *
     * @return 
     */
    Histogram& operator+=( const DataRange< D >& range )
    {
      typename DataRange< D >::const_iterator it( range.begin() );
      for( ; it != range.end(); ++it )
      {
        histogram[ it->output ]++;
        n++;
      }
      return *this;
    }

    Histogram& operator+=( const Histogram& h )
    {
      typename histogram_type::const_iterator sit = h.histogram.begin(),
        send = h.histogram.end();
      typename histogram_type::iterator it = histogram.begin();

      for( ; sit != send; ++sit, ++it )
      {
        ( *it ) += ( *sit );
      }
      n += h.n;
      return *this;
    }

    /**
     * @brief Returns mode of histogram with the corresponding
     * empirical class probability
     *
     * @return 
     */
    pair< size_t, float > getMode() const
    {
      float maxValue = FLT_MIN;
      size_t maxC;

      for( size_t i = 0; i < histogram.size(); i++ )
      {
        if( histogram[ i ] > maxValue )
        {
          maxValue = histogram[ i ];
          maxC = i;
        }
      }

      return pair< size_t, float >( maxC, maxValue / n );
    }

    /**
     * @brief Calculates entropy of the histogram
     *
     * @return 
     */
    float getEntropy() const
    {
      float entropy = 0.0f;
      typename histogram_type::const_iterator it = histogram.begin(),
        end = histogram.end();

      for( ; it != end; ++it )
      {
        if( float p_i = static_cast<float>( *it ) / n )
        {
          entropy += p_i * cvt::Math::log2( p_i );
        }
      }
      return -entropy;
    }

    float probability( size_t class_index ) const
    {
      if( !histogram[ class_index ] )
      {
        return 0.0f;
      } else {
        return static_cast<float>( histogram[ class_index ] ) / n;
      }
    }

    size_t numClasses() const
    {
      return histogram.size();
    }

    friend ostream& operator<<( ostream& os, const Histogram& s )
    {
      os << s.n << ": { ";
      typename histogram_type::const_iterator it = s.histogram.begin(),
        end = s.histogram.end();
      for( size_t c = 0; it != end; ++it, c++ )
      {
        os << "(" << c << "," << *it << ") ";
      }
      os << "}";

      return os;
    }
};

#endif
