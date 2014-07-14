#ifndef IMAGE_FEATURE_H
#define IMAGE_FEATURE_H

#include <cvt/gfx/Image.h>
#include <cvt/gfx/IMapScoped.h>

#include "ImageCommon.h"

template< size_t channels >
class ImageFeature : public FeatureBase< DataType >
{
  private:
    typedef FeatureBase< DataType > super;

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

    float operator()( const DataType& point ) const
    {
      const cvt::Image& i = point.input( channel_ );
      cvt::IMapScoped< const uint8_t > map( i );
      cvt::Vector2i p( i.width() * point1_.x, i.height() * point1_.y ),
        q( i.width() * point2_.x, i.height() * point2_.y );

      return map( p.x, p.y ) - map( q.x, q.y );
    }

    static ImageFeature get_random_feature()
    {
      cvt::Point2f p1( cvt::Math::rand( 0.0f, 1.0f ), cvt::Math::rand( 0.0f, 1.0f ) );
      cvt::Point2f p2( cvt::Math::rand( 0.0f, 1.0f ), cvt::Math::rand( 0.0f, 1.0f ) );
      size_t channel = cvt::Math::rand( 0, channels ) + 0.5f;
      return ImageFeature( p1, p2, channel );
    }

  public:
    cvt::Point2f point1_;
    cvt::Point2f point2_;
    size_t channel_;
};

#endif
