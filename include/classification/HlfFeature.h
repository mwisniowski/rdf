#ifndef IMAGE_FEATURE_H
#define IMAGE_FEATURE_H

#include <cvt/gfx/Image.h>
#include <cvt/gfx/IMapScoped.h>

#include "classification/HlfCommon.h"

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

    float operator()( const InputType& input ) const
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

    cvt::XMLNode* serialize() const
    {
      cvt::XMLElement* node = new cvt::XMLElement( "ImageFeature ");

      cvt::XMLElement* point = new cvt::XMLElement( "p1" );
      cvt::String s; 
      s.sprintf( "%f", point1_.x );
      cvt::XMLAttribute* attr = new cvt::XMLAttribute( "x", s );
      point->addChild( attr );
      s.sprintf( "%f", point1_.y );
      attr = new cvt::XMLAttribute( "y", s );
      point->addChild( attr );
      node->addChild( point );

      point = new cvt::XMLElement( "p2" );
      s.sprintf( "%f", point2_.x );
      attr = new cvt::XMLAttribute( "x", s );
      point->addChild( attr );
      s.sprintf( "%f", point2_.y );
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

      float x, y;
      cvt::XMLNode* point = node->childByName( "p1" );
      x = point->childByName( "x" )->value().toFloat();
      y = point->childByName( "y" )->value().toFloat();
      point1_ = cvt::Vector2f( x, y );

      point = node->childByName( "p2" );
      x = point->childByName( "x" )->value().toFloat();
      y = point->childByName( "y" )->value().toFloat();
      point2_ = cvt::Vector2f( x, y );
    }

  public:
    cvt::Point2f point1_;
    cvt::Point2f point2_;
    size_t channel_;
};

#endif
