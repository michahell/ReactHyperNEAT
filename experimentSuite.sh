#!/bin/sh

# make sure we have the latest updates to personal bash aliases and functions
source ~/.bashrc

# fail on anything, apparently
set -e


function notifyMsg () {
  echo '\033[0;33m' "$1" '\033[0m'
}


function errorMsg () {
  echo '\033[0;31m' "$1" '\033[0m'
}


if [ "$#" -ne 1 ]
then
  errorMsg "add experiment (folder) to make and execute as argument."
  exit
fi


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


function checkRequirements {
  # execute some which commands, parse the output to check if command exists
  command -v cmake >/dev/null 2>&1 || { echo >&2 "I require cmake but it's not installed.  Aborting."; exit 1; }
  command -v growlnotify >/dev/null 2>&1 || { echo >&2 "I require growlnotify but it's not installed.  Aborting."; exit 1; }
}


function rebuildExperimentDefinition {
  # rebuild HyperNEAT executables for selected experiment
  printf "\ec"
  notifyMsg "rebuilding HyperNEAT for selected experiment '$EXPERIMENT_NAME' "

  # added to force rebuild of hyperneat executable, picking up changes in symlinked code
  echo 'removing hyperneat executables to force rebuild...'
  rm -f HyperNEAT/NE/HyperNEAT/out/Hypercube_NEAT
  rm -f HyperNEAT/NE/HyperNEAT/out/Hypercube_NEAT_d

  echo 'removing existing experiment definition source symlinks...'
  rm -f ${PWD}/HyperNEAT/NE/HyperNEAT/Hypercube_NEAT/src/Experiments/ExperimentDefinition.cpp
  rm -f ${PWD}/HyperNEAT/NE/HyperNEAT/Hypercube_NEAT/include/Experiments/ExperimentDefinition.h

  echo 'symlinking experiment definition source into hyperneat dir...'
  ln -s ${EXPERIMENT_DEF_CPP} ${PWD}/HyperNEAT/NE/HyperNEAT/Hypercube_NEAT/src/Experiments/ExperimentDefinition.cpp
  ln -s ${EXPERIMENT_DEF_HPP} ${PWD}/HyperNEAT/NE/HyperNEAT/Hypercube_NEAT/include/Experiments/ExperimentDefinition.h

  echo 'invoking hyperneat buildAll script...'
  cd HyperNEAT && ./buildAll.sh
  cd ../
}


function rebuildExperimentControllers {
  printf "\ec"
  notifyMsg 'rebuilding controllers...'
  echo "cd into ${1}"
  cd ${1}
  # fix for Evert's mac pro having MacPorts GCC as default GCC. Comment out / remove if this is messing things up.
  # the GCC version used should be the default Xcode GCC, currently 4.2.x
  export PATH=/Applications/Webots6.3.0:/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin:/opt/X11/bin:/usr/local/munki:/usr/texbin:/usr/local/bin
  make all
  cd ../
  # remove fix for Evert's mac pro
  source ~/.bash_profile
}


function runSimulations {
  # experiment suite configuration
  SIMULATIONS=30
  PROGRESS=0

  # replace with echo, notifySingleLine is just colored output.
  printf "\ec"
  notifyMsg "starting ${SIMULATIONS} experiments for experiment '$EXPERIMENT_NAME' "
  
  for number in $(seq ${PROGRESS} ${SIMULATIONS})
  do
    # run hyperneat binary and thus experiment
    printf "\ec"
    notifyMsg "running experiment..."
    cd HyperNEAT/NE/HyperNEAT/out
    ./Hypercube_NEAT_d -R $EXPERIMENT_SEED -I $EXPERIMENT_LOCATION -O $EXPERIMENT_OUTPUTLOCATION
    cd ../../../../
    growlnotify --appIcon Webots -t 'Experiment suite' -m "simulation complete. progress: ${number} / ${SIMULATIONS}"
  done

  growlnotify --appIcon Webots -t 'Experiment suite' -m "All simulations completed."
}


function analyseResults {
  printf "\ec"
  notifyMsg 'analysing results...'
  growlnotify --appIcon Webots -t 'Experiment suite' -m "Starting analysis of data..."
  # ...TODO
  growlnotify --appIcon Webots -t 'Experiment suite' -m "Analysis of data complete."
}


# define experiment variables (callable functions)
ModNeatExperiment7 () {
  EXPERIMENT_NAME="ModNeatExperiment7"
  # hyperneat CLI required flags
  EXPERIMENT_LOCATION="${PWD}/experiment/ExperimentDefinition/ExperimentDefinitionParams.dat"
  EXPERIMENT_SEED="22"
  EXPERIMENT_OUTPUTLOCATION="${PWD}/experiment/CPPNarchive/"
  # experiment definition for symlinking into HyperNEAT
  EXPERIMENT_DEF_HPP="${PWD}/experiment/ExperimentDefinition/ExperimentDefinition.h"
  EXPERIMENT_DEF_CPP="${PWD}/experiment/ExperimentDefinition/ExperimentDefinition.cpp"
  # return $TRUE
}


# see if we can run this script
checkRequirements

# which experiment do we want to run?
ModNeatExperiment7

# list of functions to go through
# rebuildExperimentDefinition
# rebuildExperimentControllers ${1}
runSimulations
analyseResults
