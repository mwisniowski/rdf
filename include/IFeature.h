#ifndef I_FEATURE_H
#define I_FEATURE_H

template< typename D >
class IFeature 
{
  public:
    virtual float operator()( const D& point ) const =0;
};

#endif
