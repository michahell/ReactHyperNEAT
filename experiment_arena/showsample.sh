#!/bin/sh

# make sure we have the latest updates to personal bash aliases and functions
source ~/.bashrc

# colored notify output
function notifyMsg () {
  echo '\033[0;34m'"$1"'\033[0m'
}

EXPERIMENT_DIR="/Users/michahell/Documents/projects_c++/experimentSuite/experiment_arena"
SAMPLE=${EXPERIMENT_DIR}/${1}


# WEBOTS 6.x original worldfile
# WORLD="/Users/michahell/Documents/projects_c++/experiment_arena/worlds/exp7_world.wbt"

# WEBOTS 6.x original worldfile, controller names modified
WORLD="/Users/michahell/Documents/projects_c++/experimentSuite/experiment_arena/worlds/arena_world_1.wbt"

notifyMsg "launching ${1} into webots..."
notifyMsg "world: ${WORLD}"

export CPPN_XML_FILE1=$SAMPLE && webots $WORLD # --mode=realtime