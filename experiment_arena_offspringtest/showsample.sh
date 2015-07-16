#!/bin/sh

# make sure we have the latest updates to personal bash aliases and functions
source ~/.bashrc

# colored notify output
function notifyMsg () {
  echo '\033[0;34m'"$1"'\033[0m'
}

EXPERIMENT_DIR="/Users/michahell/Documents/projects_c++/experimentSuite/experiment_arena_offspringtest"
SAMPLE=${EXPERIMENT_DIR}/${1}

# WEBOTS 6.x original worldfile, controller names modified
# create a new world using world_gen.py from the template, and replace all controller names with _view:
./worlds/world_gen.py `pwd`/worlds/ view
WORLD="/Users/michahell/Documents/projects_c++/experimentSuite/experiment_arena_offspringtest/worlds/arena_world_offspringtest_view.wbt"

notifyMsg "launching ${1} into webots..."
notifyMsg "world: ${WORLD}"

export CPPN_XML_FILE1=$SAMPLE && webots $WORLD # --mode=realtime