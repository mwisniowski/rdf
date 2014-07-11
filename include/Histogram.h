#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <cvt/math/Math.h>
#include <map>

#include "Interfaces.h"

template< typename D, typename F >
class Histogram: public IStatistics< D, F, Histogram< D, F > >
{
  public:
    size_t n;

  private:
    typedef IStatistics< D, F, Histogram< D, F > >  super;
    typedef std::vector< size_t >                   histogram_type;
    histogram_type                                  histogram;

  public:
    Histogram( const typename super::ContextType& context ) :
      super( context ),
      histogram( context.num_classes ),
      n( 0 )
    {}

    Histogram( const Histogram& other ) :
      super( other ),
      histogram( other.histogram ),
      n( other.n )
    {}

    virtual ~Histogram() 
    {}

    Histogram& operator=( const Histogram& other )
    {
      if( this != &other )
      {
        this->context = other.context;
        n = other.n;
        histogram = other.histogram;
      }
      return *this;
    }

    Histogram& operator+=( const vector< size_t >& data_idxs )
    {
      vector< size_t >::const_iterator it( data_idxs.begin() );
      for( ; it != data_idxs.end(); ++it )
      {
        histogram[ this->context.data[ *it ].output ]++;
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
    pair< size_t, float > get_mode() const
    {
      float max_value = FLT_MIN;
      size_t max_c;

      for( size_t i = 0; i < histogram.size(); i++ )
      {
        if( histogram[ i ] > max_value )
        {
          max_value = histogram[ i ];
          max_c = i;
        }
      }

      return pair< size_t, float >( max_c, max_value / n );
    }

    /**
     * @brief Calculates entropy of the histogram
     *
     * @return 
     */
    float get_entropy() const
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

    // size_t num_classes() const
    // {
    //   return histogram.size();
    // }

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
