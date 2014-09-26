#ifndef DETECTION_FEATURE_H
#define DETECTION_FEATURE_H

#include <cvt/gfx/Image.h>
#include <cvt/gfx/IMapScoped.h>
#include <cvt/io/xml/XMLAttribute.h>

#include "detection/DetectionCommon.h"

class DetectionFeature : public FeatureBase< InputType >, public cvt::XMLSerializable
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
      const uint8_t i1 = *( input.map.ptr() + input.map.stride() * ( input.y + point1_.y ) + 4 * ( input.x + point1_.x ) + channel_ );
      const uint8_t i2 = *( input.map.ptr() + input.map.stride() * ( input.y + point2_.y ) + 4 * ( input.x + point2_.x ) + channel_ );
      return i1 - i2;
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

    cvt::XMLNode* serialize() const
    {
      cvt::XMLElement* node = new cvt::XMLElement( "DetectionFeature ");

      cvt::XMLElement* point = new cvt::XMLElement( "p1" );
      cvt::String s; 
      s.sprintf( "%d", point1_.x );
      cvt::XMLAttribute* attr = new cvt::XMLAttribute( "x", s );
      point->addChild( attr );
      s.sprintf( "%d", point1_.y );
      attr = new cvt::XMLAttribute( "y", s );
      point->addChild( attr );
      node->addChild( point );

      point = new cvt::XMLElement( "p2" );
      s.sprintf( "%d", point2_.x );
      attr = new cvt::XMLAttribute( "x", s );
      point->addChild( attr );
      s.sprintf( "%d", point2_.y );
      attr = new cvt::XMLAttribute( "y", s );
      point->addChild( attr );
      node->addChild( point );

      s.sprintf( "%d", channel_ );
      attr = new cvt::XMLAttribute( "channel", s );
      node->addChild( attr );
  
      return node;
    }

    void deserialize( cvt::XMLNode* node )
    {
      channel_ = node->childByName( "channel" )->value().toInteger();

      int x, y;
      cvt::XMLNode* point = node->childByName( "p1" );
      x = point->childByName( "x" )->value().toInteger();
      y = point->childByName( "y" )->value().toInteger();
      point1_ = cvt::Vector2i( x, y );

      point = node->childByName( "p2" );
      x = point->childByName( "x" )->value().toInteger();
      y = point->childByName( "y" )->value().toInteger();
      point1_ = cvt::Vector2i( x, y );
    }

  public:
    cvt::Vector2i point1_;
    cvt::Vector2i point2_;
    int channel_;
};

#endif