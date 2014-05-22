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
  string file =  "~/Developer/rdf/data/supervised classification/exp1_n2.txt";
  if( argc > 1 ) {
    file = argv[ 1 ];
  }
  ifstream is( argv[1] );

  // istream_iterator< Instance2f > start( is ), end;

  // vector< Instance2f > v( start, end );

  cout << "Starting" << endl;

  TrainingParameters params;
  params.maxDecisionLevels = 3;

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

  DataPoint2f point = data[ 2 ];
  pair< u_int, float > c = t.classify( point );

  cout << "(" << point.input[ 0 ] << "," << point.input[ 1 ] << ") classified as (" << c.first << "," << c.second << "), should be " << point.output << endl;
  cout << t;

  return 0;
}
