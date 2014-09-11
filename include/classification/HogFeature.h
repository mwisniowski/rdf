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
    float operator()( const std::vector< InputType >& input ) const
    {
      return input[ component_ ];
    }

  private:
    size_t component_;
};

#endif
