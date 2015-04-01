#!/bin/sh

# make sure we have the latest updates to personal bash aliases and functions
source ~/.bashrc

EXPERIMENT_DIR="/Users/michahell/Documents/projects_c++/experiment"
SAMPLE=${EXPERIMENT_DIR}/${1}


# WEBOTS 6.x original worldfile
# WORLD="/Users/michahell/Documents/projects_c++/experiment/worlds/exp7_world.wbt"

# WEBOTS 6.x original worldfile, controller names modified
WORLD="/Users/michahell/Documents/projects_c++/experiment/worlds/exp7_world_FINAL_1.wbt"

clear
echo "launching"
echo ${1}
echo "into webots..."

export CPPN_XML_FILE1=$SAMPLE && webots $WORLD # --mode=realtime