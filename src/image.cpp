#include <cvt/gfx/Image.h>
#include "cvt/io/FileSystem.h"

#include "ImageCommon.h"
#include "TrainingParameters.h"
#include "DataPoint.h"
#include "ImageContext.h"
#include "ForestTrainer.h"

using namespace cvt;
const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X: ", &tstruct);

    return buf;
}

size_t getData( DataRange< DataType >::collection& data,
    vector< String >& class_labels, 
    String& path )
{
  if( !path.hasSuffix( "/" ) )
  {
    path += "/";
  }

  class_labels.clear();
  FileSystem::ls( path, class_labels );

  vector< String >::iterator it = class_labels.begin();
  for( ; it != class_labels.end(); ++it )
  {
    if( !it->hasPrefix( "000" ) )
    {
      class_labels.erase( it );
    }
  }

  size_t c = 0;
  for( ; c < class_labels.size(); c++ )
  {
    String p( path + class_labels[ c ] + "/" );
    if( FileSystem::isDirectory( path ) )
    {
      vector< String > class_data;
      FileSystem::filesWithExtension( p, class_data, "png" );
      for( size_t j = 0; j < class_data.size(); j++ )
      {
        Image i;
        i.load( class_data[ j ] );
        vector< Image > v( 3 );
        i.decompose( v[ 0 ], v[ 1 ], v[ 2 ] );
        data.push_back( DataType( v, c ) );
      }
    }
  }
  return c;
}

int main(int argc, char *argv[])
{

  TrainingParameters params = {
    1, //trees
    100,  //noCandidateFeatures
    100,  //noCandidateThresholds
    15   //maxDecisionLevels
  };

  DataRange< DataType >::collection data;
  String path( argv[ 1 ] );
  cout << currentDateTime() << "Loading data" << endl;
  vector< String > class_labels;
  size_t numClasses = getData( data, class_labels, path );
  DataRange< DataType > range( data.begin(), data.end() );

  cout << currentDateTime() << "Initializing context (builds lookup table)" << endl;
  ImageContext context( params, range, numClasses );
  TrainerType trainer( context );
  cout << currentDateTime() << "Training" << endl;
  ClassifierType classifer = trainer.trainForest( range );

  cout << currentDateTime() << "Classifying" << endl;
  DataRange< DataType >::const_iterator it = data.begin(),
    end = data.end();
  float certainty = 0.0f;
  for( ; it != end; ++it )
  {
    StatisticsType s = classifer.classify( *it );
    pair< size_t, float > result = s.getMode();
    certainty += result.second;
    // cout << currentDateTime() << "(" << class_labels[ result.first ] << "," << result.second << ")" << endl;
  }
  cout << currentDateTime() << "Average certainty: " << certainty / std::distance( range.begin(), range.end() ) << endl;
  cout << currentDateTime() << "Finished" << endl;
  
  return 0;
}
