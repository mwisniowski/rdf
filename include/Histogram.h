#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <cvt/math/Math.h>
#include <map>

#include "DataRange.h"
#include "Interfaces.h"

template< typename class_type, typename D >
class Histogram: public IStatistics< D, Histogram< class_type, D > >
{
  public:
    size_t n;

  public:
    typedef std::vector< class_type >  class_set_type;
    typedef std::vector< size_t >      histogram_type;
    static class_set_type              classes;
    histogram_type                     histogram;


  public:
    Histogram() :
      n( 0 ),
      histogram( classes.size(), 0 )
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
        typename class_set_type::iterator cit = std::find( classes.begin(), classes.end(), it->output );
        if( cit == classes.end() )
        {
          classes.push_back( it->output );
          cit = classes.end() - 1 ;
          histogram.push_back( 0 );
        }
        size_t index = std::distance( classes.begin(), cit );

        histogram[ index ]++;
        n++;
      }
      return *this;
    }

    Histogram& operator+=( const Histogram& h )
    {
      histogram.resize( classes.size(), 0 );

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
    pair< class_type, float > getMode() const
    {
      float maxValue = FLT_MIN;
      class_type maxC;

      for( size_t i = 0; i < histogram.size(); i++ )
      {
        if( histogram[ i ] > maxValue )
        {
          maxValue = histogram[ i ];
          maxC = classes[ i ];
        }
      }

      return pair< class_type, float >( maxC, maxValue / n );
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

    float probability( class_type class_index ) const
    {
      typename class_set_type::iterator it = std::find( classes.begin(), classes.end(), class_index );
      if( it == classes.end() )
      {
        return 0.0f;
      } else {
        size_t index = std::distance( classes.begin(), it );
        return static_cast<float>( histogram[ index ] ) / n;
      }
    }

    size_t numClasses() const
    {
      return classes.size();
    }

    friend ostream& operator<<( ostream& os, const Histogram& s )
    {
      os << s.n << ": { ";
      typename histogram_type::const_iterator it = s.histogram.begin(),
        end = s.histogram.end();
      typename class_set_type::const_iterator cit = s.classes.begin();
      for( ; it != end; ++it, ++cit )
      {
        os << "(" << *cit << "," << *it << ") ";
      }
      os << "}";

      return os;
    }
};

template< typename class_type, typename D >
typename Histogram< class_type, D >::class_set_type Histogram< class_type, D >::classes;

#endif
