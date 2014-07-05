#include <cvt/gfx/Image.h>
#include "cvt/io/FileSystem.h"

#include "ImageCommon.h"
#include "TrainingParameters.h"
#include "DataPoint.h"
#include "ImageContext.h"
#include "ForestTrainer.h"

using namespace cvt;

void getData( DataRange< DataType >::collection& data, String& path )
{
  if( !path.hasSuffix( "/" ) )
  {
    path += "/";
  }

  vector< String > class_labels;
  FileSystem::ls( path, class_labels );

  for( size_t i = 0; i < class_labels.size(); i++ )
  {
    String class_label = class_labels[ i ];
    if( !class_label.hasPrefix( "000" ) )
    {
      continue;
    }
    String p( path + class_label + "/" );
    if( FileSystem::isDirectory( path ) )
    {
      vector< String > class_data;
      FileSystem::filesWithExtension( p, class_data, "png" );
      for( size_t j = 0; j < class_data.size(); j++ )
      {
        Image i;
        i.load( class_data[ j ] );
        vector< Image > v;
        Image r, g, b;
        i.decompose( r, g, b );
        v.push_back( r );
        v.push_back( g );
        v.push_back( b );
        data.push_back( DataType( v, class_label ) );
      }
    }
  }
}

int main(int argc, char *argv[])
{

  TrainingParameters params = {
    10, //trees
    100,  //noCandidateFeatures
    100,  //noCandidateThresholds
    100   //maxDecisionLevels
  };

  DataRange< DataType >::collection data;
  String path( argv[ 1 ] );
  cout << "Loading data" << endl;
  getData( data, path );
  cout << "Data loaded" << endl;
  DataRange< DataType > range( data.begin(), data.end() );

  cout << "Initializing context (builds lookup table)" << endl;
  ImageContext context( params, range );
  cout << "Initialized context" << endl;
  TrainerType trainer( context );
  // ClassifierType classifer = trainer.trainForest( range );
  
  return 0;
}
