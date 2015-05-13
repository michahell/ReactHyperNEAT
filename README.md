# ReactHyperNEAT

How to run an experiment:
--------------------------
run ./experimentSuite.sh <experimentfolder>


How to add a new experiment:
----------------------------

1. copy experiment_template dir and rename it (henceforth called *experimentfolder*)
2. modify *experimentfolder*/ExperimentDefinition/ExperimentDefinitionParams.dat 
3. create new webots world or copy from other experiments, rename.
4. edit *experimentfolder*/ExperimentDefinition/ExperimentDefinition.cpp
  - edit line 23 : string pathToExperiment
  - edit line 25 : string pathToWorldFile
5. edit *experimentfolder*/makefile to reflect correct paths
6. edit *experimentfolder*/showsample.sh to reflect correct paths

Issues
------

OSX 10.10 Yosemite
* no issues

OSX 10.9 Mavericks
* Unresolved _fchmodat symbol: http://ms-cheminfo.com/?q=node/90