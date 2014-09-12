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
      positive_( 0 ),
      n_( 0 ),
      sum_offset_( cvt::Vector2i( 0, 0 ) )
    {}

    DetectionStatistics( const DetectionStatistics& other ) :
      type_( other.type_ ),
      positive_( other.positive_ ),
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
        positive_ = other.positive_;
        offsets_ = other.offsets_;
        sum_offset_ = other.sum_offset_;
      }
      return *this;
    }

    DetectionStatistics& operator+=( const OutputType& output )
    {
      n_++;
      if( output.first == 1 )
      {
        positive_++;
        offsets_.push_back( output.second );
        sum_offset_ += output.second;
      }
      return *this;
    }

    DetectionStatistics& operator+=( const DetectionStatistics& h )
    {
      positive_ += h.positive_;
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
      if( positive_ == 0 || positive_ == n_ )
      {
        return 0.0f;
      }

      float p_1 = static_cast< float >( positive_ ) / n_;
      return -p_1 * cvt::Math::log2( p_1 ) - ( 1 - p_1 ) * cvt::Math::log2( 1 - p_1 );
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
      if( n_ == 0 )
      {
        return 0.0f;
      }
      float p_1 = static_cast< float >( positive_ ) / n_;
      if( class_index == 1 )
      {
        return p_1;
      }
      else
      {
        return 1 - p_1;
      }
    }

    std::pair< OutputType, float > predict() const
    {
      float x( sum_offset_.x / static_cast< float >( n_ ) );
      float y( sum_offset_.y / static_cast< float >( n_ ) );
      cvt::Vector2f mean_offset( x, y );
      return std::pair< OutputType, float >( OutputType( 1, mean_offset ), probability( 1 ) );
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
    int                           type_;
    size_t                        n_;
    size_t                        positive_;
    std::vector< cvt::Vector2i >  offsets_;
    cvt::Vector2i                 sum_offset_;
};

#endif
