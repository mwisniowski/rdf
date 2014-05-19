#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include "DataPoint.h"
#include "IDataPointCollection.h"
#include "TreeTrainer.h"
#include "TrainingParameters.h"

using namespace std;
int main(int argc, char *argv[])
{
  if( argc <= 1 )
  {
    cerr << "Path to CSV required!" << endl;
    return 1;
  }
  ifstream is( argv[1] );

  // istream_iterator< Instance2f > start( is ), end;

  // vector< Instance2f > v( start, end );

  cout << "Starting" << endl;

  TrainingParameters params;
  params.maxDecisionLevels = 4;

  cout << "Reading Data" << endl;

  istream_iterator< DataPoint2f > start( is ), end;
  IDataPointCollection data( start, end );
  is.close();
  
  cout << "Successfully read data" << endl;

  TrainingContext context;
  TreeTrainer trainer( context );

  cout << "Created trainer, starting training" << endl;

  Tree t = trainer.trainTree( params, data );

  cout << "Completed training" << endl;

  DataPoint2f point = data[ 103 ];
  pair< u_int, float > c = t.classify( point );

  cout << "(" << point.input()[ 0 ] << "," << point.input()[ 1 ] << ") classified as (" << c.first << "," << c.second << "), should be " << point.output() << endl;

  return 0;
}
