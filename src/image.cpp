#include <cvt/gfx/Image.h>

#include "TrainingParameters.h"
#include "DataPoint.h"

int main(int argc, char *argv[])
{
  TrainingParameters params = {
    100, //trees
    10,  //noCandidateFeatures
    10,  //noCandidateThresholds
    10   //maxDecisionLevels
  };

  cvt::Image i;
  i.load( argv[ 1 ] );

  vector< cvt::Image > v;
  v.push_back( i );

  DataPoint< cvt::Image, char, 1 > p( v, '1' );

  i.save( "test.png" );
}
