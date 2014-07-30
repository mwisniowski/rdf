import subprocess
import sys
import os
import glob

FEATURE_POOL = [ 10, 100, 1000, 10000 ]
FEATURES = range( 10, 101, 10 )
THRESHOLDS = [ 1, 5, 10, 50, 100 ]
DEPTHS = [ 2, 5, 10, 15, 20, 50 ]
TREES = [ 1, 2, 5, 10, 50, 100, 200 ]
FOLDS = [ 10 ]

devnull = open( os.devnull, "w" )
executable = "../build/release/toy_roc"
folder = "../data/sherwood/supervised_classification"
path = os.path.join( folder, "*.txt" )
filenames = glob.glob( path )

total = float( len( FEATURE_POOL ) * len( FEATURES ) * len( THRESHOLDS ) * len( DEPTHS ) * len( TREES ) * len( FOLDS ) * len( filenames ) )
counter = 0
for filename in filenames:
  for folds in FOLDS:
    for feature_pool_size in FEATURE_POOL:
      for features in FEATURES:
        num_features = round( ( features / float( 100 ) ) * feature_pool_size );
        if num_features <= 0:
          continue;
        for thresholds in THRESHOLDS:
          for depths in DEPTHS:
            for trees in TREES:
              args = [ str( x ) for x in [ executable, filename, num_features, thresholds, depths, trees, feature_pool_size, folds ] ]
              progress = counter / total
              print( '[ {0:20s} ] {1:8.4f}%'.format( '#' * int( progress * 20 ), progress * 100 ) )
              subprocess.call( args )
              counter += 1

