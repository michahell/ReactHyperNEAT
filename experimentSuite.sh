#!/bin/sh

# make sure we have the latest updates to personal bash aliases and functions
source ~/.bashrc

# exit on anything failing
set -e

# enable easier expressions? required for moving experiment folders into CPPNarchive/old
shopt -s extglob

# some colored output functions
function notifyMsg () {
  echo '\033[0;34m'"$1"'\033[0m'
}


function warningMsg () {
  echo '\033[0;33m'"$1"'\033[0m'
}


function errorMsg () {
  echo '\033[0;31m'"$1"'\033[0m'
}


function checkRequirements {
  # check for experiment folder argument
  if [ "$#" -ne 1 ]
  then
    errorMsg "add experiment (folder) to make and execute as argument."
    exit
  fi
  # ERRORS (ABORT)
  command -v cmake >/dev/null 2>&1 || { errorMsg >&2 "I require cmake but it's not installed.  Aborting."; exit 1; }
  command -v growlnotify >/dev/null 2>&1 || { errorMsg >&2 "I require growlnotify but it's not installed.  Aborting."; exit 1; }
  
  # TODO check all required folders and permissions
  # analysis
  # CPPNfolder
    # - old
    # - samples

  # WARNINGS
  command -v python >/dev/null 2>&1 || { warningMsg >&2 "warning: python not found ?"; }
  pip list | grep 'lxml' >/dev/null 2>&1 || { warningMsg >&2 "warning: lxml not installed ?"; }
  pip list | grep 'numpy' >/dev/null 2>&1 || { warningMsg >&2 "warning: numpy not installed ?"; }
  pip list | grep 'scipy' >/dev/null 2>&1 || { warningMsg >&2 "warning: scipy not installed ?"; }
  pip list | grep 'matplotlib' >/dev/null 2>&1 || { warningMsg >&2 "warning: matplotlib not installed ?"; }
}


function rebuildExperimentDefinition {
  # rebuild HyperNEAT executables for selected experiment
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
  notifyMsg 'rebuilding controllers...'
  cd ${1}
  # fix for Evert's mac pro having MacPorts GCC as default GCC. Comment out / remove if this is messing things up.
  # the GCC version used should be the default Xcode GCC, currently 4.2.x
  export PATH=/Applications/Webots6.3.0:/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin:/opt/X11/bin:/usr/local/munki:/usr/texbin:/usr/local/bin
  make all
  cd ../
  # remove fix for Evert's mac pro
  source ~/.bash_profile
}


function prepareExperimentFolder () {
  # making sure no other experiment CPPN folders other than 'old' and 'samples' exist
  # do this by checking the amount of folders is not greater than 2.
  notifyMsg "checking for existing CPPN experiment folders and moving them to /old..."
  cd ${1}/CPPNarchive/
  mkdir removethisfolder
  mv !(old|samples) old
  cd old/
  rm -rf removethisfolder
  cd ../../../
}


function runSimulations {
  # experiment suite configuration
  SIMULATIONS=30
  PROGRESS=0

  # replace with echo, notifySingleLine is just colored output.
  notifyMsg "starting ${SIMULATIONS} experiments for experiment '$EXPERIMENT_NAME' "
  
  for number in $(seq ${PROGRESS} ${SIMULATIONS})
  do
    # run hyperneat binary and thus experiment
    notifyMsg "running experiment ${number} ..."
    cd HyperNEAT/NE/HyperNEAT/out
    ./Hypercube_NEAT_d -R $EXPERIMENT_SEED -I $EXPERIMENT_LOCATION -O $EXPERIMENT_OUTPUTLOCATION
    cd ../../../../
    growlnotify --appIcon Webots -t 'Experiment suite' -m "simulation complete. progress: ${number} / ${SIMULATIONS}"
  done

  growlnotify --appIcon Webots -t 'Experiment suite' -m "All simulations completed."
}


function analyseResults {
  notifyMsg 'analysing results...'
  growlnotify --appIcon Webots -t 'Experiment suite' -m "Starting analysis of data..."
  cd ${1}/analysis/
  chmod +x analyseOne.py analyseAll.py
  ./analyseAll.py
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
checkRequirements ${1}

# which experiment do we want to run?
ModNeatExperiment7

# list of functions to go through
# rebuildExperimentDefinition
# rebuildExperimentControllers ${1}
# prepareExperimentFolder ${1}
# runSimulations
analyseResults ${1}
