#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <cvt/math/Math.h>
#include <map>

#include "core/Interfaces.h"
#include "detection/DetectionCommon.h"

class DetectionStatistics: public StatisticsBase< OutputType, DetectionStatistics >
{
  private:
    typedef StatisticsBase< OutputType, DetectionStatistics > super;

  public:
    DetectionStatistics() :
      type_( -1 ),
      histogram_( 2, 0 ),
      n_( 0 ),
      sum_offset_( cvt::Vector2i( 0, 0 ) )
    {}

    DetectionStatistics( const DetectionStatistics& other ) :
      type_( other.type_ ),
      histogram_( other.histogram_ ),
      n_( other.n_ ),
      offsets_( other.offsets_ ),
      sum_offset_( other.sum_offset_ )
    {}

    virtual ~DetectionStatistics() 
    {}

    DetectionStatistics& operator=( const DetectionStatistics& other )
    {
      if( this != &other )
      {
        type_ = other.type_;
        n_ = other.n_;
        histogram_ = other.histogram_;
        offsets_ = other.offsets_;
        sum_offset_ = other.sum_offset_;
      }
      return *this;
    }

    DetectionStatistics& operator+=( const OutputType& output )
    {
      histogram_[ output.first ]++;
      n_++;
      if( output.first == 1 )
      {
        offsets_.push_back( output.second );
        sum_offset_ += output.second;
      }
      return *this;
    }

    DetectionStatistics& operator+=( const DetectionStatistics& h )
    {
      std::vector< size_t >::const_iterator sit = h.histogram_.begin(),
        send = h.histogram_.end();
      std::vector< size_t >::iterator it = histogram_.begin();

      for( ; sit != send; ++sit, ++it )
      {
        ( *it ) += ( *sit );
      }
      n_ += h.n_;
      offsets_.insert( offsets_.end(), h.offsets_.begin(), h.offsets_.end() );
      sum_offset_ += h.sum_offset_;
      return *this;
    }

    /**
     * @brief Calculates entropy of the histogram
     *
     * @return 
     */
    float get_classification_entropy()
    {
      float entropy = 0.0f;
      std::vector< size_t >::const_iterator it = histogram_.begin(),
               end = histogram_.end();

      for( ; it != end; ++it )
      {
        if( float p_i = static_cast<float>( *it ) / n_ )
        {
          entropy -= p_i * cvt::Math::log2( p_i );
        }
      }
      return entropy;
    }

    float get_regression_entropy()
    {
      float entropy = 0.0f;

      float x( sum_offset_.x / static_cast< float >( n_ ) );
      float y( sum_offset_.y / static_cast< float >( n_ ) );
      cvt::Vector2f mean_offset( x, y );

      std::vector< cvt::Vector2i >::const_iterator it = offsets_.begin(),
               end = offsets_.end();
      for( ; it != end; ++it )
      {
        cvt::Vector2f v( it->x, it->y );
        entropy += ( v - mean_offset ).lengthSqr();
      }
      return entropy;
    }

    float probability( size_t class_index ) const
    {
      if( n_ > 0 )
      {
        return static_cast< float >( histogram_[ class_index ] / n_ );
      }
      else
      {
        return 0.0f;
      }
    }

    std::pair< OutputType, float > predict() const
    {
      cvt::Vector2i mean_offset = sum_offset_ / n_;
      return std::pair< OutputType, float >( OutputType( 1, mean_offset ), probability( 1 ) );
    }

    friend std::ostream& operator<<( std::ostream& os, const DetectionStatistics& s )
    {
      os << s.n_ << ": { ";
      std::vector< size_t >::const_iterator it = s.histogram_.begin(),
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

    size_t type() const
    {
      return type_;
    }

    void set_type( int type )
    {
      type_ = type;
    }

  private:
    int                            type_;
    size_t                         n_;
    std::vector< size_t >          histogram_;
    std::vector< cvt::Vector2i >   offsets_;
    cvt::Vector2i                  sum_offset_;
};

#endif
