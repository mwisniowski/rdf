#ifndef TRAINING_CONTEXT_H
#define TRAINING_CONTEXT_H

#include <cvt/math/Math.h>
#include <set>

#include "Feature.h"
#include "Histogram.h"
#include "TrainingParameters.h"

using namespace std;
using namespace cvt::Math;

class TrainingContext
{
  public:
    const size_t numClasses;
    const TrainingParameters params;

    TrainingContext( size_t classes, const TrainingParameters p ) :
      numClasses( classes ),
      params( p )
    {
      srand( time( 0 ) );
    }

    /**
     * @brief Get random unit vectors by sampling an angle from the unit circle
     *
     * @param features
     */
    void getRandomFeatures( vector< Feature >& features ) const
    {
      features.clear();
      features.reserve( params.noCandidateFeatures );
      for( size_t i = 0; i < params.noCandidateFeatures; i++ )
      {
        float angle = rand( 0.0f, TWO_PI );
        features.push_back( Feature( cvt::Math::cos( angle ), cvt::Math::sin( angle ) ) );
      }
    }

    Histogram getHistogram() const
    {
      return Histogram( numClasses );
    }

    /**
     * @brief Compute information gain by subtracting the sum of weighted child-entropies
     * from parent entropy
     *
     * @param parent_s
     * @param left_s
     * @param right_s
     *
     * @return 
     */
    float computeInformationGain( const Histogram& parent_s,
        const Histogram& left_s,
        const Histogram& right_s ) const
    {

      float H_p = parent_s.getEntropy();
      float H_l = left_s.getEntropy();
      float H_r = right_s.getEntropy();

      float fraction = left_s.n / static_cast<float>( parent_s.n );

      return H_p - ( ( fraction * H_l ) + ( ( 1.0f  - fraction ) * H_r ) );
    }

    /**
     * @brief Criterion if a leaf should be converted to split node
     *
     * @param information_gain
     *
     * @return 
     */
    bool shouldTerminate( float information_gain ) const
    {
      // TODO Magic number
      return information_gain < 0.01f;
    }
};

#endif
