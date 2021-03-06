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


# trap ctrl-c and call cleanup_before_exit()
trap cleanup_before_exit INT

function cleanup_before_exit() {
  echo ''
  warningMsg "** Trapped CTRL-C, cleaning up then exiting..."
  errorMsg "** Exiting..."
  exit
}


function checkRequirements {
  # check for experiment folder argument
  if [ "$#" -ne 1 ]
  then
    errorMsg "add experiment (folder) to make and execute as argument."
    exit
  fi
  
  notifyMsg "verifying if required software is installed..."

  # ERRORS (ABORT)
  command -v cmake >/dev/null 2>&1 || { errorMsg >&2 "I require cmake but it's not installed.  Aborting."; exit 1; }
  command -v growlnotify >/dev/null 2>&1 || { errorMsg >&2 "I require growlnotify but it's not installed.  Aborting."; exit 1; }
  
  # WARNINGS
  command -v python >/dev/null 2>&1 || { warningMsg >&2 "warning: python not found ?"; }
  pip list | grep 'lxml' >/dev/null 2>&1 || { warningMsg >&2 "warning: lxml not installed ?"; }
  pip list | grep 'numpy' >/dev/null 2>&1 || { warningMsg >&2 "warning: numpy not installed ?"; }
  pip list | grep 'scipy' >/dev/null 2>&1 || { warningMsg >&2 "warning: scipy not installed ?"; }
  pip list | grep 'matplotlib' >/dev/null 2>&1 || { warningMsg >&2 "warning: matplotlib not installed ?"; }
}


function verifyExperimentFolders {
  # check if all required folders are there. if not make them.
  notifyMsg "verifying CPPNarchive and controllers folder exist..."
  cd ${1}
  mkdir -p controllers
  mkdir -p CPPNarchive && mkdir -p CPPNarchive/old && mkdir -p CPPNarchive/samples
  cd ../
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
  source ~/.bashrc
}


function rebuildExperimentPlugins {
  notifyMsg 'rebuilding plugin(s)...'
  cd ${1}
  # fix for Evert's mac pro having MacPorts GCC as default GCC. Comment out / remove if this is messing things up.
  # the GCC version used should be the default Xcode GCC, currently 4.2.x
  export PATH=/Applications/Webots6.3.0:/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin:/opt/X11/bin:/usr/local/munki:/usr/texbin:/usr/local/bin
  cd plugins/physics/collision_detector/
  # IMPORTANT! REMOVE OLD DYLIB'S FIRST!
  rm -rf collision_detector.d collision_detector.dylib collision_detector.o
  make
  cd ../../../../
  # remove fix for Evert's mac pro
  source ~/.bash_profile
  source ~/.bashrc
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
  SIMULATIONS=5
  PROGRESS=0

  # replace with echo, notifySingleLine is just colored output.
  notifyMsg "starting ${SIMULATIONS} experiments for experiment '$EXPERIMENT_NAME' "
  
  for number in $(seq ${PROGRESS} ${SIMULATIONS})
  do
    # run hyperneat binary and thus experiment
    notifyMsg "running experiment ${number} ..."
    # create logfile
    touch ${EXPERIMENT_FOLDER}/experiment${number}.log
    cd HyperNEAT/NE/HyperNEAT/out
    # ./Hypercube_NEAT_d -R $EXPERIMENT_SEED -I $EXPERIMENT_LOCATION -O $EXPERIMENT_OUTPUTLOCATION | tee -a ${EXPERIMENT_FOLDER}/experiment${number}.log
    ./Hypercube_NEAT_d -R $RANDOM -I $EXPERIMENT_LOCATION -O $EXPERIMENT_OUTPUTLOCATION | tee -a ${EXPERIMENT_FOLDER}/experiment${number}.log
    cd ../../../../
    # growlnotify --appIcon Webots -t 'Experiment suite' -m "simulation complete. progress: ${number} / ${SIMULATIONS}"
  done

  notifyMsg "All ${SIMULATIONS} simulations completed."
  # growlnotify --appIcon Webots -t 'Experiment suite' -m "All simulations completed."
}


function analyseResults {
  notifyMsg 'analysing results...'
  growlnotify --appIcon Webots -t 'Experiment suite' -m "Starting analysis of data..."
  cd analysis
  chmod +x analyse_one.py analyse_all.py
  ./analyse_all.py ${1}
  growlnotify --appIcon Webots -t 'Experiment suite' -m "Analysis of data complete."
}



# DEFINE EXPERIMENTS

experiment_moddif () {
  EXPERIMENT_NAME="experiment moddif"
  # hyperneat CLI required flags
  EXPERIMENT_LOCATION="${PWD}/experiment/ExperimentDefinition/ExperimentDefinitionParams.dat"
  EXPERIMENT_SEED="22"
  EXPERIMENT_OUTPUTLOCATION="${PWD}/experiment/CPPNarchive/"
  # experiment definition for symlinking into HyperNEAT
  EXPERIMENT_DEF_HPP="${PWD}/experiment/ExperimentDefinition/ExperimentDefinition.h"
  EXPERIMENT_DEF_CPP="${PWD}/experiment/ExperimentDefinition/ExperimentDefinition.cpp"
}



# MINE

experiment_arena () {
  EXPERIMENT_NAME="experiment arena"
  # hyperneat CLI required flags
  EXPERIMENT_LOCATION="${PWD}/experiment_arena/ExperimentDefinition/ExperimentDefinitionParams.dat"
  EXPERIMENT_FOLDER="${PWD}/experiment_arena/"
  EXPERIMENT_SEED="24"
  EXPERIMENT_OUTPUTLOCATION="${PWD}/experiment_arena/CPPNarchive/"
  # experiment definition for symlinking into HyperNEAT
  EXPERIMENT_DEF_HPP="${PWD}/experiment_arena/ExperimentDefinition/ExperimentDefinition.h"
  EXPERIMENT_DEF_CPP="${PWD}/experiment_arena/ExperimentDefinition/ExperimentDefinition.cpp"
}

experiment_arena_substrate () {
  EXPERIMENT_NAME="experiment arena larger substrate"
  # hyperneat CLI required flags
  EXPERIMENT_LOCATION="${PWD}/experiment_arena_substrate/ExperimentDefinition/ExperimentDefinitionParams.dat"
  EXPERIMENT_FOLDER="${PWD}/experiment_arena_substrate/"
  EXPERIMENT_SEED="28"
  EXPERIMENT_OUTPUTLOCATION="${PWD}/experiment_arena_substrate/CPPNarchive/"
  # experiment definition for symlinking into HyperNEAT
  EXPERIMENT_DEF_HPP="${PWD}/experiment_arena_substrate/ExperimentDefinition/ExperimentDefinition.h"
  EXPERIMENT_DEF_CPP="${PWD}/experiment_arena_substrate/ExperimentDefinition/ExperimentDefinition.cpp"
}

experiment_arena_freezeoutput () {
  EXPERIMENT_NAME="experiment arena freeze output"
  # hyperneat CLI required flags
  EXPERIMENT_LOCATION="${PWD}/experiment_arena_freezeoutput/ExperimentDefinition/ExperimentDefinitionParams.dat"
  EXPERIMENT_FOLDER="${PWD}/experiment_arena_freezeoutput/"
  EXPERIMENT_SEED="30"
  EXPERIMENT_OUTPUTLOCATION="${PWD}/experiment_arena_freezeoutput/CPPNarchive/"
  # experiment definition for symlinking into HyperNEAT
  EXPERIMENT_DEF_HPP="${PWD}/experiment_arena_freezeoutput/ExperimentDefinition/ExperimentDefinition.h"
  EXPERIMENT_DEF_CPP="${PWD}/experiment_arena_freezeoutput/ExperimentDefinition/ExperimentDefinition.cpp"
}

experiment_arena_noinputs () {
  EXPERIMENT_NAME="experiment arena no inputs"
  # hyperneat CLI required flags
  EXPERIMENT_LOCATION="${PWD}/experiment_arena_noinputs/ExperimentDefinition/ExperimentDefinitionParams.dat"
  EXPERIMENT_FOLDER="${PWD}/experiment_arena_noinputs/"
  EXPERIMENT_SEED="32"
  EXPERIMENT_OUTPUTLOCATION="${PWD}/experiment_arena_noinputs/CPPNarchive/"
  # experiment definition for symlinking into HyperNEAT
  EXPERIMENT_DEF_HPP="${PWD}/experiment_arena_noinputs/ExperimentDefinition/ExperimentDefinition.h"
  EXPERIMENT_DEF_CPP="${PWD}/experiment_arena_noinputs/ExperimentDefinition/ExperimentDefinition.cpp"
}



# VERIFICATION EXPERIMENTS START HERE

experiment_arena_substrate_evasion () {
  EXPERIMENT_NAME="experiment arena larger substrate evasion"
  # hyperneat CLI required flags
  EXPERIMENT_LOCATION="${PWD}/experiment_arena_substrate_evasion/ExperimentDefinition/ExperimentDefinitionParams.dat"
  EXPERIMENT_FOLDER="${PWD}/experiment_arena_substrate_evasion/"
  EXPERIMENT_SEED="34"
  EXPERIMENT_OUTPUTLOCATION="${PWD}/experiment_arena_substrate_evasion/CPPNarchive/"
  # experiment definition for symlinking into HyperNEAT
  EXPERIMENT_DEF_HPP="${PWD}/experiment_arena_substrate_evasion/ExperimentDefinition/ExperimentDefinition.h"
  EXPERIMENT_DEF_CPP="${PWD}/experiment_arena_substrate_evasion/ExperimentDefinition/ExperimentDefinition.cpp"
}

experiment_arena_noinputs_evasion () {
  EXPERIMENT_NAME="experiment arena no inputs evasion"
  # hyperneat CLI required flags
  EXPERIMENT_LOCATION="${PWD}/experiment_arena_noinputs_evasion/ExperimentDefinition/ExperimentDefinitionParams.dat"
  EXPERIMENT_FOLDER="${PWD}/experiment_arena_noinputs_evasion/"
  EXPERIMENT_SEED="36"
  EXPERIMENT_OUTPUTLOCATION="${PWD}/experiment_arena_noinputs_evasion/CPPNarchive/"
  # experiment definition for symlinking into HyperNEAT
  EXPERIMENT_DEF_HPP="${PWD}/experiment_arena_noinputs_evasion/ExperimentDefinition/ExperimentDefinition.h"
  EXPERIMENT_DEF_CPP="${PWD}/experiment_arena_noinputs_evasion/ExperimentDefinition/ExperimentDefinition.cpp"
}

experiment_arena_freezeoutput_evasion () {
  EXPERIMENT_NAME="experiment arena freeze output evasion"
  # hyperneat CLI required flags
  EXPERIMENT_LOCATION="${PWD}/experiment_arena_freezeoutput_evasion/ExperimentDefinition/ExperimentDefinitionParams.dat"
  EXPERIMENT_FOLDER="${PWD}/experiment_arena_freezeoutput_evasion/"
  EXPERIMENT_SEED="38"
  EXPERIMENT_OUTPUTLOCATION="${PWD}/experiment_arena_freezeoutput_evasion/CPPNarchive/"
  # experiment definition for symlinking into HyperNEAT
  EXPERIMENT_DEF_HPP="${PWD}/experiment_arena_freezeoutput_evasion/ExperimentDefinition/ExperimentDefinition.h"
  EXPERIMENT_DEF_CPP="${PWD}/experiment_arena_freezeoutput_evasion/ExperimentDefinition/ExperimentDefinition.cpp"
}

experiment_arena_evasion () {
  EXPERIMENT_NAME="experiment arena evasion"
  # hyperneat CLI required flags
  EXPERIMENT_LOCATION="${PWD}/experiment_arena_evasion/ExperimentDefinition/ExperimentDefinitionParams.dat"
  EXPERIMENT_FOLDER="${PWD}/experiment_arena_evasion/"
  EXPERIMENT_SEED="40"
  EXPERIMENT_OUTPUTLOCATION="${PWD}/experiment_arena_evasion/CPPNarchive/"
  # experiment definition for symlinking into HyperNEAT
  EXPERIMENT_DEF_HPP="${PWD}/experiment_arena_evasion/ExperimentDefinition/ExperimentDefinition.h"
  EXPERIMENT_DEF_CPP="${PWD}/experiment_arena_evasion/ExperimentDefinition/ExperimentDefinition.cpp"
}



# TEST EXPERIMENTS

experiment_arena_offspringtest () {
  EXPERIMENT_NAME="experiment arena offspringtest"
  # hyperneat CLI required flags
  EXPERIMENT_LOCATION="${PWD}/experiment_arena_offspringtest/ExperimentDefinition/ExperimentDefinitionParams.dat"
  EXPERIMENT_FOLDER="${PWD}/experiment_arena_offspringtest/"
  EXPERIMENT_SEED="34"
  EXPERIMENT_OUTPUTLOCATION="${PWD}/experiment_arena_offspringtest/CPPNarchive/"
  # experiment definition for symlinking into HyperNEAT
  EXPERIMENT_DEF_HPP="${PWD}/experiment_arena_offspringtest/ExperimentDefinition/ExperimentDefinition.h"
  EXPERIMENT_DEF_CPP="${PWD}/experiment_arena_offspringtest/ExperimentDefinition/ExperimentDefinition.cpp"
}


# see if we can run this script
checkRequirements ${1}

# which experiment do we want to run?

# experiment_moddif

# OBSTACLE STEPPING
# experiment_arena
# experiment_arena_noinputs
# experiment_arena_substrate
# experiment_arena_freezeoutput

# TESTING
experiment_arena_offspringtest

# BETTER EVASION VALIDATION
# experiment_arena_substrate_evasion
# experiment_arena_noinputs_evasion
# experiment_arena_freezeoutput_evasion
# experiment_arena_evasion

# list of functions to go through
verifyExperimentFolders ${1}
# rebuildExperimentDefinition
rebuildExperimentControllers ${1}
# rebuildExperimentPlugins ${1}
# prepareExperimentFolder ${1}
# runSimulations ${1}
# analyseResults ${1}
