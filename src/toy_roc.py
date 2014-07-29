import subprocess
import sys
import os
import glob

FEATURE_POOL = [ 2**x for x in range( 1,13 ) ]
FEATURES = range( 10, 101, 10 )
THRESHOLDS = [ 1 ] + range( 10, 100, 10 )
DEPTHS = range( 2, 20 )
TREES = [ 2**x for x in range( 0, 9 ) ]
FOLDS = [ 3, 5, 10 ]

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
                            print( '\r[ {0:20s} ] {1:8.4f}%'.format( '#' * int( progress * 20 ), progress * 100 ) ),
                            subprocess.call( args, stdout=devnull, stderr=subprocess.STDOUT )
                            counter += 1
            
        
