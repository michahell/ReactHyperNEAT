#!/bin/sh

# make sure we have the latest updates to personal bash aliases and functions
source ~/.bashrc

# define experiment variables as callable functions
ModNeatExperiment7 () {
  EXPERIMENT_NAME="ModNeatExperiment7"
  EXPERIMENT_LOCATION="/Users/michahell/Documents/projects_c++/experimentSuite/HyperNEAT/ModNeatExperiment7/ModNeatExperiment7.dat"
  EXPERIMENT_SEED="22"
  return $TRUE
}

# TODO start selected experiment
ModNeatExperiment7

# general variables
OUTPUTLOCATION="/Users/michahell/Documents/projects_c++/experimentSuite/experiment/CPPNarchive/"

# replace with echo, notifySingleLine is just colored output.
notifySingleLine "starting experiment '$EXPERIMENT_NAME' "

# run hyperneat binary
cd ../HyperNEAT/NE/HyperNEAT/out/
# replace with echo, notifySingleLine is just colored output.
notifySingleLine "running HyperNEAT..."
./Hypercube_NEAT_d -R $EXPERIMENT_SEED -I $EXPERIMENT_LOCATION -O $OUTPUTLOCATION
