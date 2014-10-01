#ifndef TOY_FEATURE_H
#define TOY_FEATURE_H

#include <vector>
#include <cvt/gfx/Image.h>
#include <cvt/gfx/IMapScoped.h>

#include "classification/HogCommon.h"

class HogFeature: public FeatureBase< InputType >
{
  private:
    typedef FeatureBase< InputType > super;

  public:
    HogFeature()
    {}

    HogFeature( size_t component ) :
      component_( component )
    {}

    HogFeature( const HogFeature& other ) :
      component_( other.component_ )
    {}

    virtual ~HogFeature()
    {}

    HogFeature& operator=( const HogFeature& other )
    {
      if( this != &other )
      {
        component_ = other.component_;
      }
      return *this;
    }

    /**
     * @brief Dot product of point and feature vector
     *
     * @param point
     *
     * @return 
     */
    float operator()( const InputType& input ) const
    {
      return input[ component_ ];
    }

    friend std::ostream& operator<<( std::ostream& os, const HogFeature& f )
    {
      os << "[ " << f.component_ << " ]";
      return os;
    }

    cvt::XMLNode* serialize() const
    {
      cvt::XMLElement* node = new cvt::XMLElement( "HogFeature ");

      cvt::String s; 
      s.sprintf( "%d", component_ );
      cvt::XMLAttribute* attr = new cvt::XMLAttribute( "component", s );
      node->addChild( attr );

      return node;
    }

    void deserialize( cvt::XMLNode* node )
    {
      component_ = node->childByName( "component" )->value().toInteger();
    }

  private:
    size_t component_;
};

#endif
