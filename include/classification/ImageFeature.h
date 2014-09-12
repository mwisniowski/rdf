#ifndef IMAGE_FEATURE_H
#define IMAGE_FEATURE_H

#include <cvt/gfx/Image.h>
#include <cvt/gfx/IMapScoped.h>

#include "classification/ImageCommon.h"

class ImageFeature : public FeatureBase< InputType >
{
  private:
    typedef FeatureBase< InputType > super;

  public:
    ImageFeature()
    {}

    ImageFeature( const ImageFeature& other ) :
      super( other ),
      point1_( other.point1_ ),
      point2_( other.point2_ ),
      channel_( other.channel_ )
    {}

    ImageFeature( const cvt::Point2f& point1_, const cvt::Point2f& point2_, size_t channel_ ) :
      point1_( point1_ ),
      point2_( point2_ ),
      channel_( channel_ )
    {}

    virtual ~ImageFeature() 
    {}

    ImageFeature& operator=( const ImageFeature& other )
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

    friend std::ostream& operator<<( std::ostream& os, const ImageFeature& f )
    {
      os << "[" << f.point1_ << ", " << f.point2_ << ", " << f.channel_ << " ]";
      return os;
    }

  public:
    cvt::Point2f point1_;
    cvt::Point2f point2_;
    size_t channel_;
};

#endif
