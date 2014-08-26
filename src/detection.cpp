#include <sstream>
#include <regex>

#include <cvt/gfx/Image.h>
#include <cvt/io/FileSystem.h>

#include "helper/gnuplot_i.hpp"

#include "detection/DetectionContext.h"

void get_data( std::vector< DataType >& data,
    cvt::String& path )
{
  if( !path.hasSuffix( "/" ) )
  {
    path += "/";
  }

  std::vector< cvt::String > annotations;
  cvt::FileSystem::ls( path, annotations );

  const std::regex filename_rgx( "\"([^\"]+)\": " );
  const std::regex rect_rgx( "\\((-?\\d+), (-?\\d+), (-?\\d+), (-?\\d+)\\)" );
  const std::regex coord_rgx( "-?\\d+" );

  std::vector< cvt::String >::iterator it = annotations.begin();
  for( ; it != annotations.end(); ++it )
  {
    if( !it->hasSuffix( "idl" ) )
    {
      continue;
    }

    std::string line;
    std::ifstream file( ( path + *it  ).c_str() );
    std::cout << "Processing IDL file: " << path + *it << std::endl;
    while( std::getline( file, line ) )
    {
      std::smatch filename_match;
      std::regex_search( line, filename_match, filename_rgx );
      std::string data_filename( filename_match[ 1 ] );

      cvt::Image img;
      img.load( path + cvt::String( data_filename.c_str() ) );
      img.convert( img, cvt::IFormat::GRAY_UINT16 );
      std::cout << "Processing image file: " << data_filename << std::endl;

      std::vector< cvt::Recti > rects;
      const std::sregex_token_iterator end;
      std::vector< int > vec { 1, 2, 3, 4 };
      std::sregex_token_iterator ri( line.begin(), line.end(), rect_rgx, vec );
      int count = 0;
      while( ri != end )
      {
        int x = atoi( ri->str().c_str() ); ++ri;
        int y = atoi( ri->str().c_str() ); ++ri;
        int width = atoi( ri->str().c_str() ) - x; ++ri;
        int height = atoi( ri->str().c_str() ) - y; ++ri;
        rects.push_back( cvt::Recti( x, y, width, height ) );
        count++;
      }
      std::cout << "Found " << count << " rectangles" << std::endl;

      std::vector< cvt::Vector2i > centers;
      for( size_t i = 0; i < rects.size(); i++ )
      {
        const cvt::Recti& r = rects[ i ];
        int x( cvt::Math::round( r.x + r.width/2.0f ) );
        int y( cvt::Math::round( r.y + r.height/2.0f ) );
        centers.push_back( cvt::Vector2i( x, y ) );
      } 

      count = 0;
      const size_t border = ( PATCH_SIZE - 1 ) / 2;
      for( size_t y = border; y < img.height() - border; y++ )
      {
        for( size_t x = PATCH_SIZE; x < img.width() - PATCH_SIZE; x++ )
        {
          cvt::Recti roi( x - border, y - border, PATCH_SIZE, PATCH_SIZE );
          cvt::Image patch( img, &roi, 0 );
          std::vector< cvt::Image > patch_vector( 1, patch );
          // std::vector< cvt::Image > channels( 3 );
          // patch.decompose( channels[ 0 ], channels[ 1 ], channels[ 2 ] );

          int rect_idx = -1;
          for( size_t r = 0; r < rects.size() && rect_idx < 0; r++ )
          {
            const cvt::Recti& rect = rects[ r ];
            if( rect.contains( x, y ) )
            {
              rect_idx = r;
            }
          }
          if( rect_idx < 0 )
          {
            data.push_back( DataType( patch_vector, std::make_pair( 0, cvt::Vector2i( 0, 0 ) ) ) );
          } else
          {
            data.push_back( DataType( patch_vector, std::make_pair( 1, centers[ rect_idx ] ) ) );
          }
          count++;
        }
      }
      std::cout << "Generated " << count << " data points" << std::endl;
    }
    file.close();
  }
}

int main(int argc, char *argv[])
{
  std::cout << "##########     Starting     ##########" << std::endl;

  srand( time( NULL ) );
  TrainingParameters params = {
    1, //trees
    100,  //no_candidate_features
    100,  //no_candidate_thresholds
    15,   //max_decision_levels
    3000
  };

  float split = 0.3;
  if( argc > 2 ) params.no_candidate_features = atoi( argv[ 2 ] );
  if( argc > 3 ) params.no_candate_thresholds = atoi( argv[ 3 ] );
  if( argc > 4 ) params.max_decision_levels = atoi( argv[ 4 ] );
  if( argc > 5 ) params.trees = atoi( argv[ 5 ] );
  if( argc > 6 ) params.pool_size = atoi( argv[ 6 ] );
  if( argc > 7 ) split = atof( argv[ 7 ] );

  std::cout << "Parameters:" << std::endl;
  std::cout << "  features="   << params.no_candidate_features << std::endl;
  std::cout << "  thresholds=" << params.no_candate_thresholds << std::endl;
  std::cout << "  depth="      << params.max_decision_levels << std::endl;
  std::cout << "  trees="      << params.trees << std::endl;
  std::cout << "  pool_size="  << params.pool_size << std::endl;
  std::cout << "  split="      << split << std::endl;
  std::cout << "  path="       << argv[ 1 ] << std::endl;

  std::vector< DataType > data;
  cvt::String path( argv[ 1 ] );
  std::cout << "Loading data" << std::endl;

  get_data( data, path );
  std::random_shuffle( data.begin(), data.end() );

  size_t n = data.size() * split;
  std::vector< DataType > training_data( data.begin(), data.end() - n );
  std::vector< DataType > testing_data( data.end() - n, data.end() );

  std::cout << "Initializing context (builds lookup table)" << std::endl;
  DetectionContext context( params, training_data );
  std::cout << "Training" << std::endl;

  ClassifierType classifier;
  TrainerType::train( classifier, context );

  // std::cout << "Classifying" << std::endl;
  // std::vector< std::vector< size_t > > confusion_matrix;
  // for( size_t i = 0; i < num_classes; i++ )
  // {
  //   confusion_matrix.push_back( std::vector< size_t >( num_classes, 0 ) );
  // }
  // for( size_t i = 0; i < testing_data.size(); i++ )
  // {
  //   const StatisticsType s = classifier.classify( context, testing_data[ i ] );
  //   confusion_matrix[ s.get_mode().first ][ testing_data[ i ].output() ]++;
  // }
  //
  // std::cout << "Statistics:" << std::endl;
  // std::vector< double > plot_x, plot_y;
  // float acc = 0.0f;
  // for( size_t c = 0; c < num_classes; c++ )
  // {
  //   size_t true_positive = confusion_matrix[ c ][ c ];
  //
  //   size_t labelled_positive = 0;
  //   size_t classified_positive = 0;
  //   for( size_t cc = 0; cc < num_classes; cc++ )
  //   {
  //     labelled_positive += confusion_matrix[ cc ][ c ];
  //     classified_positive += confusion_matrix[ c ][ cc ];
  //   }
  //   size_t labelled_negative = n - labelled_positive;
  //
  //   size_t false_positive = classified_positive - true_positive;
  //   float fpr = static_cast< float >( false_positive ) / labelled_negative;
  //   float tpr = static_cast< float >( true_positive ) / labelled_positive;
  //   acc += true_positive;
  //
  //   plot_x.push_back( fpr );
  //   plot_y.push_back( tpr );
  //
  //   std::cout << "  Class " << c << ": (" << fpr << ", " << tpr << ")" << std::endl;
  // }
  // acc /= n;
  // std::cout << "  Accuracy: " << acc << std::endl;
  //
  // try
  // {
  //   Gnuplot g;
  //   std::ostringstream os;
  //   os <<
  //     "features="    << params.no_candidate_features <<
  //     " thresholds=" << params.no_candate_thresholds <<
  //     " depth="      << params.max_decision_levels   <<
  //     " trees="      << params.trees                 <<
  //     " pool_size="  << params.pool_size             <<
  //     " split="      << split                        <<
  //     " path="       << path;
  //   g.set_title( os.str() );
  //   g.set_xlabel("False positive rate");
  //   g.set_ylabel("True positive rate");
  //   g << "set size square";
  //
  //   g << "set xtics .1";
  //   g << "set ytics .1";
  //   g << "set mxtics 2";
  //   g << "set mytics 2";
  //   g.set_xrange(0,1);
  //   g.set_yrange(0,1);
  //   g.set_grid();
  //
  //   g.unset_legend();
  //   g.set_style("lines lt -1").plot_slope(1.0f,0.0f,"Random");
  //   g.set_style("points lt 3").plot_xy( plot_x, plot_y );
  // } catch( GnuplotException e )
  // {
  //   std::cout << e.what();
  // }
  //
  // std::cout << "Finished" << std::endl;
  //
  // getchar();

  return 0;
}
