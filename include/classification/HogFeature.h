#ifndef HOG_FEATURE_H
#define HOG_FEATURE_H

#include <vector>

#include "classification/HogCommon.h"

template< size_t d >
class HogFeature: public FeatureBase< InputType >
{
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

    friend std::ostream& operator<<( std::ostream& os, const HogFeature& feature )
    {
      os << feature.component_;
      return os;
    }

    cvt::XMLNode* serialize() const
    {
      cvt::XMLElement* node = new cvt::XMLElement( "HogFeature ");
      cvt::String s; 
      s.sprintf( "%d", component_ );
      cvt::XMLAttribute* att = new cvt::XMLAttribute( "component", s );

      // cvt::XMLElement* v = new cvt::XMLElement( "v" );
      // cvt::String s; 
      // s.sprintf( "%d", v_.size() );
      // cvt::XMLAttribute* attr = new cvt::XMLAttribute( "size", s );
      // for( size_t i = 0; i < v_.size(); i++ )
      // {
      //   cvt::XMLElement* elem = new cvt::XMLElement( "elem" );
      //   s.sprintf( "%d", i );
      //   attr = new cvt::XMLAttribute( "idx", s );
      //   elem->addChild( attr );
      //   s.sprintf( "%f", v_[ i ] );
      //   attr = new cvt::XMLAttribute( "value", s );
      //   elem->addChild( attr );
      // }

      return node;
    }

    void deserialize( cvt::XMLNode* node )
    {
      component_ = node->childByName( "component" )->value().toInteger();
      // cvt::XMLNode* v = node->childByName( "v" );
      // size_t size = v->childByName( "size" )->value().toInteger();
      // v_.resize( size, 0.0f );
      // for( size_t i = 0; i < size; i++ )
      // {
      //   cvt::XMLNode* elem = v->child( i );
      //   if( elem->name() != "elem" )
      //   {
      //     continue;
      //   }
      //   size_t idx = elem->childByName( "idx" )->value().toInteger();
      //   v_[ idx ] = elem->childByName( "value" )->value().toFloat();
      // }
    }

  private:
    size_t component_;
};

#endif
