#ifndef DETECTION_FEATURE_H
#define DETECTION_FEATURE_H

#include <cvt/gfx/Image.h>
#include <cvt/gfx/IMapScoped.h>

#include "detection/DetectionCommon.h"

class DetectionFeature : public FeatureBase< InputType >
{
  private:
    typedef FeatureBase< InputType > super;

  public:
    DetectionFeature()
    {}

    DetectionFeature( const DetectionFeature& other ) :
      point1_( other.point1_ ),
      point2_( other.point2_ ),
      channel_( other.channel_ )
    {}

    DetectionFeature( const cvt::Point2f& point1_, const cvt::Point2f& point2_, size_t channel_ ) :
      point1_( point1_ ),
      point2_( point2_ ),
      channel_( channel_ )
    {}

    virtual ~DetectionFeature() 
    {}

    DetectionFeature& operator=( const DetectionFeature& other )
    {
      if( this != &other )
      {
        point1_ = other.point1_;
        point2_ = other.point2_;
        channel_ = other.channel_;
      }
      return *this;
    }

    float operator()( const std::vector< InputType >& input ) const
    {
      const cvt::Image& i = input[ channel_ ];
      cvt::IMapScoped< const uint8_t > map( i );
      cvt::Vector2i p( i.width() * point1_.x, i.height() * point1_.y ),
        q( i.width() * point2_.x, i.height() * point2_.y );

      return map( p.x, p.y ) - map( q.x, q.y );
    }

  public:
    cvt::Point2f point1_;
    cvt::Point2f point2_;
    size_t channel_;
};

#endif
