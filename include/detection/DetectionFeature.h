#ifndef DETECTION_FEATURE_H
#define DETECTION_FEATURE_H

#include <cvt/gfx/Image.h>
#include <cvt/gfx/IMapScoped.h>
#include <cvt/io/xml/XMLAttribute.h>

#include "detection/DetectionCommon.h"

class DetectionFeature : public FeatureBase< InputType >
{
  private:
    typedef FeatureBase< InputType > super;

  public:
    DetectionFeature() :
      channel_( -1 )
    {}

    DetectionFeature( const DetectionFeature& other ) :
      point1_( other.point1_ ),
      point2_( other.point2_ ),
      channel_( other.channel_ )
    {}

    DetectionFeature( const cvt::Point2f& point1_, const cvt::Point2f& point2_, int channel_ ) :
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

    float operator()( const InputType& input ) const
    {
      const cvt::IMapScoped< const uint8_t > map( input.channels[ channel_ ] );
      return map( input.x + point1_.x, input.y + point1_.y ) - map( input.x + point2_.x, input.y + point2_.y );
    }

    friend std::ostream& operator<<( std::ostream& os, const DetectionFeature& f )
    {
      if( f.channel_ < 0 )
      {
        os << "n/a";
      }
      else
      {
        os << "[" << f.point1_ << ", " << f.point2_ << ", " << f.channel_ << " ]";
      }
      return os;
    }

    cvt::XMLNode* serialize() const;

    void deserialize( cvt::XMLNode* node );

  public:
    cvt::Vector2i point1_;
    cvt::Vector2i point2_;
    int channel_;
};

#endif
