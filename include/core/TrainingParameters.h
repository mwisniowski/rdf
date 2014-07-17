#ifndef TRAINING_PARAMETERS_H
#define TRAINING_PARAMETERS_H

struct TrainingParameters
{
  unsigned int trees;
  unsigned int no_candidate_features;
  unsigned int no_candate_thresholds;
  unsigned int max_decision_levels;
  unsigned int pool_size;
};

#endif
