#ifndef IMAGE_FEATURE_H
#define IMAGE_FEATURE_H

#include <cvt/gfx/Image.h>
#include <cvt/gfx/IMapScoped.h>

#include "ImageCommon.h"

class ImageFeature : public IFeature< DataType >
{
  private:
    typedef IFeature< DataType > super;

  public:
    cvt::Point2f p1;
    cvt::Point2f p2;
    size_t channel;

  public:
    ImageFeature()
    {}

    ImageFeature( const ImageFeature& other ) :
      super( other ),
      p1( other.p1 ),
      p2( other.p2 ),
      channel( other.channel )
    {}

    ImageFeature( const cvt::Point2f& p, const cvt::Point2f& q, size_t c ) :
      p1( p ),
      p2( q ),
      channel( c )
    {}

    virtual ~ImageFeature() 
    {}

    ImageFeature& operator=( const ImageFeature& other )
    {
      if( this != &other )
      {
        super::operator=( other );
        p1 = other.p1;
        p2 = other.p2;
        channel = other.channel;
      }
      return *this;
    }

    float operator()( const DataType& point ) const
    {
      const cvt::Image& i = point.input[ channel ];
      cvt::IMapScoped< const float > map( i );
      cvt::Vector2i p( i.width() * p1.x, i.height() * p2.y ),
        q( i.width() * p2.x, i.height() * p2.y );

      return map( p.x, p.y ) - map( q.x, q.y );
    }
};

#endif
