#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <cvt/math/Math.h>
#include <map>

#include "Interfaces.h"

template< typename D, typename F >
class Histogram: public StatisticsBase< D, F, Histogram< D, F > >
{
  private:
    typedef StatisticsBase< D, F, Histogram< D, F > >  super;

  public:
    Histogram( TrainingContextBase< D, F, Histogram< D, F > >& context ) :
      super( context ),
      histogram_( context.num_classes() ),
      n_( 0 )
    {}

    Histogram( const Histogram& other ) :
      super( other ),
      histogram_( other.histogram_ ),
      n_( other.n_ )
    {}

    virtual ~Histogram() 
    {}

    Histogram& operator=( const Histogram& other )
    {
      if( this != &other )
      {
        super::operator=( other );
        n_ = other.n_;
        histogram_ = other.histogram_;
      }
      return *this;
    }

    Histogram& operator+=( const std::vector< size_t >& data_idxs )
    {
      std::vector< size_t >::const_iterator it( data_idxs.begin() );
      for( ; it != data_idxs.end(); ++it )
      {
        const D& d = this->context_.data_point( *it );
        histogram_[ d.output() ]++;
        n_++;
      }
      return *this;
    }

    Histogram& operator+=( const Histogram& h )
    {
      typename std::vector< size_t >::const_iterator sit = h.histogram_.begin(),
        send = h.histogram_.end();
      typename std::vector< size_t >::iterator it = histogram_.begin();

      for( ; sit != send; ++sit, ++it )
      {
        ( *it ) += ( *sit );
      }
      n_ += h.n_;
      return *this;
    }

    /**
     * @brief Returns mode of histogram with the corresponding
     * empirical class probability
     *
     * @return 
     */
    std::pair< size_t, float > get_mode() const
    {
      float max_value = FLT_MIN;
      size_t max_c;

      for( size_t i = 0; i < histogram_.size(); i++ )
      {
        if( histogram_[ i ] > max_value )
        {
          max_value = histogram_[ i ];
          max_c = i;
        }
      }

      return std::pair< size_t, float >( max_c, max_value / n_ );
    }

    /**
     * @brief Calculates entropy of the histogram
     *
     * @return 
     */
    float get_entropy() const
    {
      float entropy = 0.0f;
      typename std::vector< size_t >::const_iterator it = histogram_.begin(),
        end = histogram_.end();

      for( ; it != end; ++it )
      {
        if( float p_i = static_cast<float>( *it ) / n_ )
        {
          entropy += p_i * cvt::Math::log2( p_i );
        }
      }
      return -entropy;
    }

    float probability( size_t class_index ) const
    {
      if( !histogram_[ class_index ] )
      {
        return 0.0f;
      } else {
        return static_cast<float>( histogram_[ class_index ] ) / n_;
      }
    }

    friend std::ostream& operator<<( std::ostream& os, const Histogram& s )
    {
      os << s.n << ": { ";
      typename std::vector< size_t >::const_iterator it = s.histogram_.begin(),
        end = s.histogram_.end();
      for( size_t c = 0; it != end; ++it, c++ )
      {
        os << "(" << c << "," << *it << ") ";
      }
      os << "}";

      return os;
    }

    size_t n() const
    {
      return n_;
    }

  private:
    size_t                 n_;
    std::vector< size_t >  histogram_;
};

#endif
