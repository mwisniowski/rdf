import subprocess
import sys
import os
import glob
import datetime

class Tee(object):
    def __init__(self, name, mode):
        self.file = open(name, mode)
        self.stdout = sys.stdout
        sys.stdout = self
    def __del__(self):
        sys.stdout = self.stdout
        self.file.close()
    def write(self, data):
        self.file.write(data)
        self.stdout.write(data)

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
# filenames = glob.glob( path )
filenames = [ "../data/sherwood/supervised_classification/exp4_n4.txt", "../data/sherwood/supervised_classification/exp5_n4.txt" ]
tee = Tee( "toy_roc.log", "w" )

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
              timestamp_before = datetime.datetime.now()
              proc = subprocess.Popen( args, stdout=subprocess.PIPE )

              for line in proc.stdout:
                tee.write( line )
              proc.wait()

              timestamp_after = datetime.datetime.now()
              progress = counter / total
              tee.write( "Elapsed: {}\n".format( timestamp_after - timestamp_before ) )
              tee.write( '[ {0:20s} ] {1:8.4f}%\n'.format( '#' * int( progress * 20 ), progress * 100 ) )

              counter += 1

