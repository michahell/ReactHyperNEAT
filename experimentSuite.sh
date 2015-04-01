#!/bin/sh

# make sure we have the latest updates to personal bash aliases and functions
source ~/.bashrc

# Author : Teddy Skarin
# 1. Create ProgressBar function
# 1.1 Input is currentState($1) and totalState($2)
function ProgressBar {
  # Process data
  let _progress=(${1}*100/${2}*100)/100
  let _done=(${_progress}*4)/10
  let _left=40-$_done
  # Build progressbar string lengths
  _done=$(printf "%${_done}s")
  _left=$(printf "%${_left}s")

  # 1.2 Build progressbar strings and print the ProgressBar line
  # 1.2.1 Output example:
  # 1.2.1.1 Progress : [########################################] 100%
  printf "\rProgress : [${_done// /#}${_left// /-}] ${_progress}%%"
}

# experiment suite configuration
SIMULATIONS=30
PROGRESS=0
ROOTDIR=${PWD}

# define experiment variables as callable functions
ModNeatExperiment7 () {
  EXPERIMENT_NAME="ModNeatExperiment7"
  EXPERIMENT_LOCATION="/Users/michahell/Documents/projects_c++/experimentSuite/experiment/HyperNEAT/ModNeatExperiment7/ModNeatExperiment7.dat"
  EXPERIMENT_SEED="22"
  EXPERIMENT_OUTPUTLOCATION="/Users/michahell/Documents/projects_c++/experimentSuite/experiment//CPPNarchive/"
  return $TRUE
}

TestExperiment () {
  EXPERIMENT_NAME="TestExperiment"
  EXPERIMENT_LOCATION="${ROOTDIR}/experiment_test/HyperNEAT/TestExperiment/TestExperiment.dat"
  EXPERIMENT_SEED="23"
  EXPERIMENT_OUTPUTLOCATION="${ROOTDIR}/experiment_test/CPPNarchive/"
  return $TRUE
}

# define selected experiments variables
# TestExperiment
ModNeatExperiment7

#echo $EXPERIMENT_LOCATION
#echo $EXPERIMENT_SEED
#echo $EXPERIMENT_OUTPUTLOCATION

# rebuild HyperNEAT executables for selected experiment
notifySingleLine "rebuilding HyperNEAT for selected experiment '$EXPERIMENT_NAME' "

# added to force rebuid of hyperneat executable, picking up changes in symlinked code
# rm -f ${ROOTDIR}/HyperNEAT/NE/HyperNEAT/out/Hypercube_NEAT
# rm -f ${ROOTDIR}/HyperNEAT/NE/HyperNEAT/out/Hypercube_NEAT_d
# added to force rebuid of hyperneat executable, picking up changes in symlinked code

# replace with echo, notifySingleLine is just colored output.
notifySingleLine "starting experiment suite for experiment '$EXPERIMENT_NAME' "


# run simulation suite
# for number in $(seq ${PROGRESS} ${SIMULATIONS})
# do
#   # run hyperneat binary and thus experiment
#   # notifySingleLine "running experiment..."
#   cd HyperNEAT/NE/HyperNEAT/out
#   ./Hypercube_NEAT_d -R $EXPERIMENT_SEED -I $EXPERIMENT_LOCATION -O $EXPERIMENT_OUTPUTLOCATION
#   cd ../../../../

#   # update progress bar
#   sleep 0.1
#   let PROGRESS=PROGRESS+1
#   ProgressBar ${number} ${SIMULATIONS}
#   growlnotify --appIcon Webots -t 'Experiment suite' -m "simulation complete. progress: ${number} / ${SIMULATIONS}"
  
# done

# growlnotify --appIcon Webots -t 'Experiment suite' -m "All simulations completed. starting analysis..."

# replace with echo, notifySingleLine is just colored output.
# notifySingleLine "rebuilding experiment... '$EXPERIMENT_NAME' "

# replace experiment definition CPP and H symlinks in the hyperNEAT library experiment folder
# cd /Users/michahell/Documents/projects_c++/experimentSuite/HyperNEAT/NE/HyperNEAT/Hypercube_NEAT/include/Experiments
